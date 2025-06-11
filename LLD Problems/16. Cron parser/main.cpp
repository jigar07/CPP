#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <memory>
using namespace std;

// ---- Custom Exception ----
class InvalidInputException : public runtime_error {
public:
    InvalidInputException(const string& msg = "Invalid Input")
        : runtime_error(msg) {}
};

// ---- Enum for Component Types ----
enum class CronComponentType {
    MINUTE,
    HOUR,
    DAY_OF_MONTH,
    MONTH,
    DAY_OF_WEEK,
    COMMAND
};

// ---- Range Class ----
class Range {
    int min_;
    int max_;
public:
    Range(int min, int max) : min_(min), max_(max) {}

    bool contains(int value) const {
        return value >= min_ && value <= max_;
    }

    int getMin() const { return min_; }
    int getMax() const { return max_; }
};

// ---- IComponentTypeData Interface ----
class IComponentTypeData {
public:
    virtual ~IComponentTypeData() = default;
};

// ---- ComponentTypeDataList Class ----
class ComponentTypeDataList : public IComponentTypeData {
    vector<int> values_;
public:
    ComponentTypeDataList(const vector<int>& values) : values_(values) {}

    const vector<int>& getValues() const {
        return values_;
    }
};

// ---- ComponentTypeDataString Class ----
class ComponentTypeDataString : public IComponentTypeData {
    string value_;
public:
    ComponentTypeDataString(const string& value) : value_(value) {}

    const string& getValue() const {
        return value_;
    }
};

// ---- ComponentTypeData Class ----
class ComponentTypeData {
    CronComponentType type_;
    Range validRange_;
public:
    ComponentTypeData(CronComponentType type, const Range& range)
        : type_(type), validRange_(range) {}

    CronComponentType getType() const { return type_; }
    const Range& getValidRange() const { return validRange_; }
};

// ---- CronComponent Class ----
class CronComponent {
    CronComponentType type_;
    unique_ptr<IComponentTypeData> data_;
public:
    CronComponent(CronComponentType type, unique_ptr<IComponentTypeData> data)
        : type_(type), data_(move(data)) {}

    CronComponentType getType() const { return type_; }
    const IComponentTypeData* getData() const { return data_.get(); }
};

// ---- CronExpression Class ----
class CronExpression {
    vector<CronComponent> components_;
public:
    CronExpression(vector<CronComponent> components)
        : components_(move(components)) {}

    const vector<CronComponent>& getComponents() const {
        return components_;
    }
};

// ---- IExpressionTypeParser Interface ----
class IExpressionTypeParser {
public:
    virtual ~IExpressionTypeParser() = default;

    virtual unique_ptr<IComponentTypeData> parse(const string& expr) = 0;
    virtual bool doesSupport(const string& expr) = 0;
    virtual bool isValid(const string& expr, const Range& range) = 0;
};

// ---- SimpleValueExpressionParser ----
class SimpleValueExpressionParser : public IExpressionTypeParser {
public:
    unique_ptr<IComponentTypeData> parse(const std::string& expr) override {
        int value = std::stoi(expr);
        return make_unique<ComponentTypeDataList>(vector<int>{ value });
    }

    bool doesSupport(const std::string& expr) override {
        return expr.find(',') == string::npos && expr.find('-') == string::npos;
    }

    bool isValid(const std::string& expr, const Range& range) override {
        int value = std::stoi(expr);
        return range.contains(value);
    }
};

// ---- CommaExpressionTypeParser ----
class CommaExpressionTypeParser : public IExpressionTypeParser {
public:
    unique_ptr<IComponentTypeData> parse(const string& expr) override {
        vector<int> values;
        stringstream ss(expr);
        string token;

        while (getline(ss, token, ',')) {
            values.push_back(stoi(token));
        }

        return make_unique<ComponentTypeDataList>(values);
    }

    bool doesSupport(const string& expr) override {
        return expr.find(',') != string::npos;
    }

    bool isValid(const string& expr, const Range& range) override {
        stringstream ss(expr);
        string token;
        bool found = false;

        while (getline(ss, token, ',')) {
            token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
            if (token.empty()) continue;

            int value = stoi(token);
            if (!range.contains(value)) return false;
            found = true;
        }

        if (!found) throw InvalidInputException("Empty comma-separated list");
        return true;
    }
};

// ---- HyphenExpressionTypeParser ----
class HyphenExpressionTypeParser : public IExpressionTypeParser {
public:
    unique_ptr<IComponentTypeData> parse(const string& expr) override {
        size_t dashPos = expr.find('-');
        int start = stoi(expr.substr(0, dashPos));
        int end = stoi(expr.substr(dashPos + 1));
        vector<int> values;

        for (int i = start; i <= end; ++i) {
            values.push_back(i);
        }

        return make_unique<ComponentTypeDataList>(values);
    }

    bool doesSupport(const string& expr) override {
        return expr.find('-') != string::npos;
    }

    bool isValid(const string& expr, const Range& range) override {
        size_t dashPos = expr.find('-');
        if (dashPos == string::npos) return false;

        int start = stoi(expr.substr(0, dashPos));
        int end = stoi(expr.substr(dashPos + 1));
        return range.contains(start) && range.contains(end) && start <= end;
    }
};

// ---- CronParser ----
class CronParser {
    vector<unique_ptr<IExpressionTypeParser>> parsers_;
    map<int, ComponentTypeData> componentTypeDataMap_;

public:
    CronParser(vector<unique_ptr<IExpressionTypeParser>> parsers,
               map<int, ComponentTypeData> map)
        : parsers_(move(parsers)), componentTypeDataMap_(move(map)) {}

    CronExpression parse(const string& cronExpression) {
        istringstream ss(cronExpression);
        vector<string> parts;
        string token;
        while (ss >> token) {
            parts.push_back(token);
        }

        if (parts.size() != 6) {
            throw InvalidInputException("Cron expression must have 6 parts.");
        }

        vector<CronComponent> parsedComponents;

        for (int i = 0; i < 5; ++i) {
            const string& part = parts[i];
            bool parsed = false;

            for (const auto& parser : parsers_) {
                if (parser->doesSupport(part)) {
                    const ComponentTypeData& typeData = componentTypeDataMap_.at(i);
                    if (parser->isValid(part, typeData.getValidRange())) {
                        auto data = parser->parse(part);
                        parsedComponents.emplace_back(typeData.getType(), move(data));
                        parsed = true;
                        break;
                    }
                }
            }

            if (!parsed) {
                throw InvalidInputException("Unsupported or invalid format: " + part);
            }
        }

        // Add command part
        auto commandData = make_unique<ComponentTypeDataString>(parts[5]);
        parsedComponents.emplace_back(CronComponentType::COMMAND, move(commandData));

        return CronExpression(move(parsedComponents));
    }
};

// ---- CronService ----
class CronService {
    CronParser parser_;

public:
CronService()
        : parser_(
            []() {
                vector<unique_ptr<IExpressionTypeParser>> parsers;
                parsers.push_back(make_unique<CommaExpressionTypeParser>());
                parsers.push_back(make_unique<HyphenExpressionTypeParser>());
                parsers.push_back(make_unique<SimpleValueExpressionParser>());
                return parsers;
            }(),
            {
                {0, ComponentTypeData(CronComponentType::MINUTE, Range(0, 59))},
                {1, ComponentTypeData(CronComponentType::HOUR, Range(0, 23))},
                {2, ComponentTypeData(CronComponentType::DAY_OF_MONTH, Range(1, 31))},
                {3, ComponentTypeData(CronComponentType::MONTH, Range(1, 12))},
                {4, ComponentTypeData(CronComponentType::DAY_OF_WEEK, Range(0, 6))}
            }
        ) {}

    void parseAndPrint(const string& expr) {
        try {
            CronExpression result = parser_.parse(expr);
            const auto& components = result.getComponents();
            for (const auto& comp : components) {
                cout << "Component: ";
                switch (comp.getType()) {
                    case CronComponentType::MINUTE: cout << "Minute"; break;
                    case CronComponentType::HOUR: cout << "Hour"; break;
                    case CronComponentType::DAY_OF_MONTH: cout << "DayOfMonth"; break;
                    case CronComponentType::MONTH: cout << "Month"; break;
                    case CronComponentType::DAY_OF_WEEK: cout << "DayOfWeek"; break;
                    case CronComponentType::COMMAND: cout << "Command"; break;
                }
                cout << " -> ";

                const auto* data = comp.getData();
                if (const auto* list = dynamic_cast<const ComponentTypeDataList*>(data)) {
                    for (int val : list->getValues()) {
                        cout << val << " ";
                    }
                } else if (const auto* str = dynamic_cast<const ComponentTypeDataString*>(data)) {
                    cout << str->getValue();
                }

                cout << "\n";
            }
        } catch (const exception& e) {
            cerr << "Error parsing cron expression: " << e.what() << "\n";
        }
    }
};

// ---- Main ----
int main() {
    CronService service;
    string expression = "1,15 0 1,32 1,4 1-5 /usr/bin/find";
    // string expression = "1,15 0 1,15 1,4 1-5 /usr/bin/find";
    service.parseAndPrint(expression);

    return 0;
}
