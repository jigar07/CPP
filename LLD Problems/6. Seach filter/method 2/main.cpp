#include <iostream>
#include <string>
#include <variant>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <limits>
using namespace std;

// Forward declare FilterValue for recursive use
struct FilterValue;

// Recursive map alias
using FilterMap = map<string, FilterValue>;

// FilterValue variant: can be string, double, or nested FilterMap
struct FilterValue : variant<string, double, FilterMap> {
    using variant::variant; // inherit constructors

    // Helper to get string if holds string
    const string* getString() const {
        return get_if<string>(this);
    }
    // Helper to get double if holds double
    const double* getDouble() const {
        return get_if<double>(this);
    }
    // Helper to get FilterMap if holds FilterMap
    const FilterMap* getMap() const {
        return get_if<FilterMap>(this);
    }
};

// Product struct
struct Product {
    string id;
    string name;
    double price;
    string category;
};

// Interface for filtering criteria
struct IFilteringCriteria {
    virtual ~IFilteringCriteria() = default;

    virtual bool doesSupport(const string& filterName) const = 0;

    virtual bool doesProductMatch(const Product& product, const FilterValue& filterData) const = 0;
};

// Category filter
class CategoryFilteringCriteria : public IFilteringCriteria {
public:
    bool doesSupport(const string& filterName) const override {
        return filterName == "CATEGORY";
    }

    bool doesProductMatch(const Product& product, const FilterValue& filterData) const override {
        if (const string* val = filterData.getString()) {
            return product.category == *val;
        }
        return false;
    }
};

// Price filter (expects map with "min" and/or "max")
class PriceFilteringCriteria : public IFilteringCriteria {
public:
    bool doesSupport(const string& filterName) const override {
        return filterName == "PRICE";
    }

    bool doesProductMatch(const Product& product, const FilterValue& filterData) const override {
        if (const FilterMap* pmap = filterData.getMap()) {
            double minVal = 0.0;
            double maxVal = numeric_limits<double>::max();

            auto itMin = pmap->find("min");
            if (itMin != pmap->end()) {
                if (const double* val = itMin->second.getDouble()) {
                    minVal = *val;
                }
            }
            auto itMax = pmap->find("max");
            if (itMax != pmap->end()) {
                if (const double* val = itMax->second.getDouble()) {
                    maxVal = *val;
                }
            }

            return product.price >= minVal && product.price <= maxVal;
        }
        return false;
    }
};

// Logical AND filter
class AndFilteringCriteria : public IFilteringCriteria {
public:
    bool doesSupport(const string& filterName) const override {
        return filterName == "AND";
    }

    bool doesProductMatch(const Product& product, const FilterValue& filterData) const override;
};

// Logical OR filter
class OrFilteringCriteria : public IFilteringCriteria {
public:
    bool doesSupport(const string& filterName) const override {
        return filterName == "OR";
    }

    bool doesProductMatch(const Product& product, const FilterValue& filterData) const override;
};

// Logical NOT filter
class NotFilteringCriteria : public IFilteringCriteria {
public:
    bool doesSupport(const string& filterName) const override {
        return filterName == "NOT";
    }

    bool doesProductMatch(const Product& product, const FilterValue& filterData) const override;
};

// FilterFactory singleton
class FilterFactory {
    vector<shared_ptr<IFilteringCriteria>> criteriaList;

public:
    FilterFactory() {
        criteriaList.emplace_back(make_shared<CategoryFilteringCriteria>());
        criteriaList.emplace_back(make_shared<PriceFilteringCriteria>());
        criteriaList.emplace_back(make_shared<AndFilteringCriteria>());
        criteriaList.emplace_back(make_shared<OrFilteringCriteria>());
        criteriaList.emplace_back(make_shared<NotFilteringCriteria>());
    }

    static FilterFactory& getInstance() {
        static FilterFactory instance;
        return instance;
    }

    bool doesProductMatch(const Product& product, const FilterValue& filter) {
        if (const FilterMap* pmap = filter.getMap()) {
            if (pmap->size() != 1) {
                throw runtime_error("FilterMap must contain exactly one filter");
            }
            const auto& [filterName, filterData] = *pmap->begin();

            for (auto& crit : criteriaList) {
                if (crit->doesSupport(filterName)) {
                    return crit->doesProductMatch(product, filterData);
                }
            }
            throw runtime_error("No criteria found for filter: " + filterName);
        }
        throw runtime_error("Filter must be a FilterMap");
    }
};

// Implement AND filter logic
bool AndFilteringCriteria::doesProductMatch(const Product& product, const FilterValue& filterData) const {
    if (const FilterMap* pmap = filterData.getMap()) {
        auto itLeft = pmap->find("left");
        auto itRight = pmap->find("right");
        if (itLeft == pmap->end() || itRight == pmap->end()) {
            throw runtime_error("AND filter requires 'left' and 'right'");
        }
        return FilterFactory::getInstance().doesProductMatch(product, itLeft->second) &&
               FilterFactory::getInstance().doesProductMatch(product, itRight->second);
    }
    throw runtime_error("Invalid data for AND filter");
}

// Implement OR filter logic
bool OrFilteringCriteria::doesProductMatch(const Product& product, const FilterValue& filterData) const {
    if (const FilterMap* pmap = filterData.getMap()) {
        auto itLeft = pmap->find("left");
        auto itRight = pmap->find("right");
        if (itLeft == pmap->end() || itRight == pmap->end()) {
            throw runtime_error("OR filter requires 'left' and 'right'");
        }
        return FilterFactory::getInstance().doesProductMatch(product, itLeft->second) ||
               FilterFactory::getInstance().doesProductMatch(product, itRight->second);
    }
    throw runtime_error("Invalid data for OR filter");
}

// Implement NOT filter logic
bool NotFilteringCriteria::doesProductMatch(const Product& product, const FilterValue& filterData) const {
    if (const FilterMap* pmap = filterData.getMap()) {
        auto itOperand = pmap->find("operand");
        if (itOperand == pmap->end()) {
            throw runtime_error("NOT filter requires 'operand'");
        }
        return !FilterFactory::getInstance().doesProductMatch(product, itOperand->second);
    }
    throw runtime_error("Invalid data for NOT filter");
}

// Main for testing
int main() {
    vector<Product> products = {
        {"1", "iPhone", 99.99, "phone"},
        {"2", "Samsung", 149.99, "phone"},
        {"3", "Dell Laptop", 499.99, "computer"},
        {"4", "Nokia", 49.99, "phone"},
        {"5", "Sony TV", 299.99, "electronics"}
    };

    // Build a complex filter:
    // (PRICE between 50 and 100 AND CATEGORY = "phone") OR NOT(CATEGORY = "electronics")
    FilterMap filter = {
        {"OR", FilterMap{
            {"left", FilterMap{
                {"AND", FilterMap{
                    {"left", FilterMap{
                        {"PRICE", FilterMap{{"min", 50.0}, {"max", 100.0}}}
                    }},
                    {"right", FilterMap{
                        {"CATEGORY", string("phone")}
                    }}
                }}
            }},
            {"right", FilterMap{
                {"NOT", FilterMap{
                    {"operand", FilterMap{
                        {"CATEGORY", string("electronics")}
                    }}
                }}
            }}
        }}
    };

    auto& factory = FilterFactory::getInstance();

    cout << "Products matching filter:\n";
    for (const auto& p : products) {
        try {
            if (factory.doesProductMatch(p, filter)) {
                cout << " - " << p.name << "\n";
            }
        } catch (const exception& ex) {
            cerr << "Filter error: " << ex.what() << "\n";
        }
    }

    return 0;
}