#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <algorithm>
#include <stdexcept>
using namespace std;

// ------------------ Product ------------------
struct Product {
    string id;
    string name;
    double amount;
    string category;
};

// // ------------------ IFilterTypeData ------------------
// struct IFilterTypeData {
//     virtual ~IFilterTypeData() = default;
// };

// // ------------------ BooleanFilterTypeData ------------------
// struct BooleanFilterTypeData : public IFilterTypeData {
//     map<string, variant<string, double, map<string, variant<string, double>>>> left;
//     map<string, variant<string, double, map<string, variant<string, double>>>> right;
// };

// // ------------------ NotFilterTypeData ------------------
// struct NotFilterTypeData : public IFilterTypeData {
//     map<string, variant<string, double, map<string, variant<string, double>>>> operand;
// };

// // ------------------ NameFilterTypeData ------------------
// struct NameFilterTypeData : public IFilterTypeData {
//     string value;
//     string matchWay;
// };

// ------------------ IStringMatchingStrategy ------------------
struct IStringMatchingStrategy {
    virtual bool isMatching(const string& a, const string& b) const = 0;
};

struct EqualsStringMatchingStrategy : public IStringMatchingStrategy {
    bool isMatching(const string& a, const string& b) const override {
        return a == b;
    }
};

struct PrefixStringMatchingStrategy : public IStringMatchingStrategy {
    bool isMatching(const string& a, const string& b) const override {
        return a.rfind(b, 0) == 0;
    }
};

// ------------------ IFilteringCriteria ------------------
struct IFilteringCriteria {
    virtual ~IFilteringCriteria() = default;
    virtual bool doesSupport(const string& filterName) const = 0;
    virtual bool doesProductMatch(const Product& product, const variant<string, double, map<string, variant<string, double>>>& filterData) const = 0;
};

// ------------------ CATEGORY ------------------
struct CategoryFilteringCriteria : public IFilteringCriteria {
    bool doesSupport(const string& filterName) const override {
        return filterName == "CATEGORY";
    }

    bool doesProductMatch(const Product& product, const variant<string, double, map<string, variant<string, double>>>& filterData) const override {
        if (holds_alternative<string>(filterData)) {
            return product.category == get<string>(filterData);
        }
        return false;
    }
};

// ------------------ PRICE ------------------
struct PriceFilteringCriteria : public IFilteringCriteria {
    bool doesSupport(const string& filterName) const override {
        return filterName == "PRICE";
    }

    bool doesProductMatch(const Product& product, const variant<string, double, map<string, variant<string, double>>>& filterData) const override {
        if (holds_alternative<map<string, variant<string, double>>>(filterData)) {
            auto mapData = get<map<string, variant<string, double>>>(filterData);
            double minValue = get<double>(mapData["min"]);
            double maxValue = get<double>(mapData["max"]);
            return product.amount >= minValue && product.amount <= maxValue;
        }
        return false;
    }
};

// ------------------ NAME ------------------
struct NameFilteringCriteria : public IFilteringCriteria {
    map<string, shared_ptr<IStringMatchingStrategy>> stringMatchingStrategyMap;

    NameFilteringCriteria() {
        stringMatchingStrategyMap["EQUALS"] = make_shared<EqualsStringMatchingStrategy>();
        stringMatchingStrategyMap["PREFIX"] = make_shared<PrefixStringMatchingStrategy>();
    }

    bool doesSupport(const string& filterName) const override {
        return filterName == "NAME";
    }

    bool doesProductMatch(const Product& product, const variant<string, double, map<string, variant<string, double>>>& filterData) const override {
        if (holds_alternative<map<string, variant<string, double>>>(filterData)) {
            auto mapData = get<map<string, variant<string, double>>>(filterData);
            string value = get<string>(mapData.at("value"));
            string matchWay = get<string>(mapData.at("matchWay"));

            auto strategy = stringMatchingStrategyMap.find(matchWay);
            if (strategy == stringMatchingStrategyMap.end()) {
                throw runtime_error("Unsupported string match strategy");
            }
            return strategy->second->isMatching(product.name, value);
        }
        return false;
    }
};

// ------------------ InventoryFilter ------------------
class InventoryFilter {
private:
    map<string, shared_ptr<IFilteringCriteria>> filteringCriteriasMap;

public:
    InventoryFilter() {
        filteringCriteriasMap["CATEGORY"] = make_shared<CategoryFilteringCriteria>();
        filteringCriteriasMap["PRICE"] = make_shared<PriceFilteringCriteria>();
        filteringCriteriasMap["NAME"] = make_shared<NameFilteringCriteria>();
    }

    vector<Product> filter(const vector<Product>& allProds,
                                const map<string, variant<string, double, map<string, variant<string, double>>>>& filterProps,
                                const string& logicType) {
        vector<Product> filteredList;

        for (const auto& product : allProds) {
            bool doesMatch = (logicType == "and");

            for (const auto& [key, val] : filterProps) {
                auto it = filteringCriteriasMap.find(key);
                if (it == filteringCriteriasMap.end()) {
                    throw runtime_error("No valid filtering criteria found for key: " + key);
                }
                // it->first is a key
                bool match = it->second->doesProductMatch(product, val);

                if (logicType == "or") {
                    if (match) {
                        doesMatch = true;
                        break;
                    }
                } else if (logicType == "and") {
                    if (!match) {
                        doesMatch = false;
                        break;
                    }
                }
            }

            if (doesMatch) {
                filteredList.push_back(product);
            }
        }

        return filteredList;
    }
};

int main() {
    InventoryFilter filter;
    vector<Product> products = {
        {"1", "iPhone", 99.0, "phone"},
        {"2", "Galaxy", 150.0, "phone"},
        {"3", "Pixel", 89.0, "electronics"}
    };

    // variant:
    // variant is a type-safe union introduced in C++17. It allows a variable to hold one value out of a set of predefined types at a time. Think of it as a safer, modern alternative to union, but with type checking and no undefined behavior if used correctly.
    map<string, variant<string, double, map<string, variant<string, double>>>> filterProps;
    filterProps["CATEGORY"] = string("phone");
    filterProps["PRICE"] = map<string, variant<string, double>>{
        {"min", 100.0},
        {"max", 200.0}
    };

    auto filtered = filter.filter(products, filterProps, "and");

    for (const auto& p : filtered) {
        cout << "Matched: " << p.name << "\n";
    }

    return 0;
}
