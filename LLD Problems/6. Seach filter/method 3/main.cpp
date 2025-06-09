#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <limits>

// --- Product Definition ---
struct Product {
    std::string id;
    std::string name;
    double price;
    std::string category;
};

// --- Interface for all filtering criteria ---
class IFilteringCriteria {
public:
    virtual ~IFilteringCriteria() = default;
    virtual bool doesProductMatch(const Product& product) const = 0;
};

// --- Leaf Filters ---

class CategoryFilteringCriteria : public IFilteringCriteria {
    std::string categoryToMatch;
public:
    explicit CategoryFilteringCriteria(std::string category) 
        : categoryToMatch(std::move(category)) {}

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
    std::shared_ptr<IFilteringCriteria> left;
    std::shared_ptr<IFilteringCriteria> right;
public:
    AndFilteringCriteria(std::shared_ptr<IFilteringCriteria> l, std::shared_ptr<IFilteringCriteria> r)
        : left(std::move(l)), right(std::move(r)) {}

    bool doesProductMatch(const Product& product) const override {
        return left->doesProductMatch(product) && right->doesProductMatch(product);
    }
};

class OrFilteringCriteria : public IFilteringCriteria {
    std::shared_ptr<IFilteringCriteria> left;
    std::shared_ptr<IFilteringCriteria> right;
public:
    OrFilteringCriteria(std::shared_ptr<IFilteringCriteria> l, std::shared_ptr<IFilteringCriteria> r)
        : left(std::move(l)), right(std::move(r)) {}

    bool doesProductMatch(const Product& product) const override {
        return left->doesProductMatch(product) || right->doesProductMatch(product);
    }
};

class NotFilteringCriteria : public IFilteringCriteria {
    std::shared_ptr<IFilteringCriteria> operand;
public:
    explicit NotFilteringCriteria(std::shared_ptr<IFilteringCriteria> op)
        : operand(std::move(op)) {}

    bool doesProductMatch(const Product& product) const override {
        return !operand->doesProductMatch(product);
    }
};

// --- Helper function to filter a list of products ---
std::vector<Product> filterProducts(
    const std::vector<Product>& products,
    const IFilteringCriteria& criteria)
{
    std::vector<Product> result;
    for (const auto& p : products) {
        if (criteria.doesProductMatch(p)) {
            result.push_back(p);
        }
    }
    return result;
}

// --- Example Usage ---
int main() {
    std::vector<Product> products = {
        {"1", "iPhone", 999.0, "phone"},
        {"2", "Galaxy", 899.0, "phone"},
        {"3", "MacBook", 1999.0, "laptop"},
        {"4", "Dell XPS", 1399.0, "laptop"},
        {"5", "Nokia", 49.0, "phone"},
        {"6", "Logitech Mouse", 25.0, "accessory"}
    };

    // Create some leaf criteria
    auto priceBetween50And1000 = std::make_shared<PriceFilteringCriteria>(50, 1000);
    auto categoryPhone = std::make_shared<CategoryFilteringCriteria>("phone");

    // Compose: (priceBetween50And1000 AND categoryPhone)
    auto priceAndCategory = std::make_shared<AndFilteringCriteria>(priceBetween50And1000, categoryPhone);

    // Compose: NOT (categoryPhone)
    auto notPhone = std::make_shared<NotFilteringCriteria>(categoryPhone);

    // Compose: (priceBetween50And1000 AND categoryPhone) OR NOT (categoryPhone)
    auto complexFilter = OrFilteringCriteria(priceAndCategory, notPhone);

    auto filtered = filterProducts(products, complexFilter);

    std::cout << "Filtered products:\n";
    for (const auto& p : filtered) {
        std::cout << "  " << p.name << " (" << p.category << "), price: " << p.price << "\n";
    }

    return 0;
}
