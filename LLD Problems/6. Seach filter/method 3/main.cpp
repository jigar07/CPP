#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <limits>
using namespace std;

// --- Product Definition ---
struct Product {
    string id;
    string name;
    double price;
    string category;
};

// --- Interface for all filtering criteria ---
class IFilteringCriteria {
public:
    virtual ~IFilteringCriteria() = default;
    virtual bool doesProductMatch(const Product& product) const = 0;
};

// --- Leaf Filters ---

class CategoryFilteringCriteria : public IFilteringCriteria {
    string categoryToMatch;
public:
    explicit CategoryFilteringCriteria(string category) 
        : categoryToMatch(move(category)) {}

    bool doesProductMatch(const Product& product) const override {
        return product.category == categoryToMatch;
    }
};

class PriceFilteringCriteria : public IFilteringCriteria {
    double minPrice;
    double maxPrice;
public:
    PriceFilteringCriteria(double minP, double maxP)
        : minPrice(minP), maxPrice(maxP) {}

    bool doesProductMatch(const Product& product) const override {
        return product.price >= minPrice && product.price <= maxPrice;
    }
};

// --- Compound Filters ---

class AndFilteringCriteria : public IFilteringCriteria {
    shared_ptr<IFilteringCriteria> left;
    shared_ptr<IFilteringCriteria> right;
public:
    AndFilteringCriteria(shared_ptr<IFilteringCriteria> l, shared_ptr<IFilteringCriteria> r)
        : left(move(l)), right(move(r)) {}

    bool doesProductMatch(const Product& product) const override {
        return left->doesProductMatch(product) && right->doesProductMatch(product);
    }
};

class OrFilteringCriteria : public IFilteringCriteria {
    shared_ptr<IFilteringCriteria> left;
    shared_ptr<IFilteringCriteria> right;
public:
    OrFilteringCriteria(shared_ptr<IFilteringCriteria> l, shared_ptr<IFilteringCriteria> r)
        : left(move(l)), right(move(r)) {}

    bool doesProductMatch(const Product& product) const override {
        return left->doesProductMatch(product) || right->doesProductMatch(product);
    }
};

class NotFilteringCriteria : public IFilteringCriteria {
    shared_ptr<IFilteringCriteria> operand;
public:
    explicit NotFilteringCriteria(shared_ptr<IFilteringCriteria> op)
        : operand(move(op)) {}

    bool doesProductMatch(const Product& product) const override {
        return !operand->doesProductMatch(product);
    }
};

// --- Helper function to filter a list of products ---
vector<Product> filterProducts(
    const vector<Product>& products,
    const IFilteringCriteria& criteria)
{
    vector<Product> result;
    for (const auto& p : products) {
        if (criteria.doesProductMatch(p)) {
            result.push_back(p);
        }
    }
    return result;
}

// --- Example Usage ---
int main() {
    vector<Product> products = {
        {"1", "iPhone", 999.0, "phone"},
        {"2", "Galaxy", 899.0, "phone"},
        {"3", "MacBook", 1999.0, "laptop"},
        {"4", "Dell XPS", 1399.0, "laptop"},
        {"5", "Nokia", 49.0, "phone"},
        {"6", "Logitech Mouse", 25.0, "accessory"}
    };

    // Create some leaf criteria
    auto priceBetween50And1000 = make_shared<PriceFilteringCriteria>(50, 1000);
    auto categoryPhone = make_shared<CategoryFilteringCriteria>("phone");

    // Compose: (priceBetween50And1000 AND categoryPhone)
    auto priceAndCategory = make_shared<AndFilteringCriteria>(priceBetween50And1000, categoryPhone);

    // Compose: NOT (categoryPhone)
    auto notPhone = make_shared<NotFilteringCriteria>(categoryPhone);

    // Compose: (priceBetween50And1000 AND categoryPhone) OR NOT (categoryPhone)
    auto complexFilter = OrFilteringCriteria(priceAndCategory, notPhone);

    auto filtered = filterProducts(products, complexFilter);

    cout << "Filtered products:\n";
    for (const auto& p : filtered) {
        cout << "  " << p.name << " (" << p.category << "), price: " << p.price << "\n";
    }

    return 0;
}
