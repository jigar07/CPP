#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

// ===== DOMAIN ENTITIES =====

class User {
public:
    std::string id;
    User(const std::string& id) : id(id) {}
};

class Seller {
public:
    std::string id;
    Seller(const std::string& id) : id(id) {}
};

class Product {
public:
    std::string id;
    double price;
    Seller* seller;
    Product(const std::string& id, double price, Seller* seller) 
        : id(id), price(price), seller(seller) {}
};

class TimeRange {
public:
    std::time_t start;
    std::time_t end;
    TimeRange(std::time_t start, std::time_t end) : start(start), end(end) {}

    bool isLive() const {
        std::time_t now = std::time(nullptr);
        return now >= start && now <= end;
    }

    bool isInPast() const {
        return std::time(nullptr) > end;
    }
};

enum class OfferType {
    PERCENT_DISCOUNT,
    FLAT_DISCOUNT
};

enum class CouponApplicableTo {
    SELLER,
    PRODUCT,
    GLOBAL
};

class Offer {
public:
    CouponApplicableTo applicableTo;
    std::vector<Seller*> applicableSellers;
    std::vector<Product*> applicableProducts;
    OfferType offerType;
    double offerPercent;
    double offerFlat;

    Offer(CouponApplicableTo applicableTo, OfferType offerType, double offerPercent = 0.0, double offerFlat = 0.0)
        : applicableTo(applicableTo), offerType(offerType), offerPercent(offerPercent), offerFlat(offerFlat) {}
};

class Coupon {
public:
    TimeRange validity;
    std::string code;
    Offer offer;
    bool isDeleted;
    User* createdBy;

    Coupon(const TimeRange& validity, const std::string& code, const Offer& offer, User* createdBy)
        : validity(validity), code(code), offer(offer), isDeleted(false), createdBy(createdBy) {}

    bool isActive() const {
        return !isDeleted && validity.isLive();
    }
};

class Cart {
public:
    User* user;
    std::vector<Product*> productList;

    Cart(User* user) : user(user) {}

    double getTotalPrice() const {
        double total = 0.0;
        for (const auto& product : productList) {
            total += product->price;
        }
        return total;
    }
};

// ===== INTERFACES =====

class ICouponMatchRule {
public:
    virtual bool isApplicable(const Coupon& coupon, const Cart& cart) const = 0;
    virtual bool apply(const Coupon& coupon, const Cart& cart) const = 0;
    virtual ~ICouponMatchRule() = default;
};

class ICouponRewardRule {
public:
    virtual bool isApplicable(OfferType offerType) const = 0;
    virtual double apply(const Offer& offer, double amount) const = 0;
    virtual ~ICouponRewardRule() = default;
};

// ===== MATCH RULES =====

class GlobalCouponMatchRule : public ICouponMatchRule {
public:
    bool isApplicable(const Coupon& coupon, const Cart&) const override {
        return coupon.offer.applicableTo == CouponApplicableTo::GLOBAL;
    }

    bool apply(const Coupon&, const Cart&) const override {
        return true;
    }
};

class SellerCouponMatchRule : public ICouponMatchRule {
public:
    bool isApplicable(const Coupon& coupon, const Cart&) const override {
        return coupon.offer.applicableTo == CouponApplicableTo::SELLER;
    }

    bool apply(const Coupon& coupon, const Cart& cart) const override {
        for (Product* product : cart.productList) {
            if (std::find(coupon.offer.applicableSellers.begin(),
                          coupon.offer.applicableSellers.end(),
                          product->seller) == coupon.offer.applicableSellers.end()) {
                return false;
            }
        }
        return true;
    }
};

class ProductCouponMatchRule : public ICouponMatchRule {
public:
    bool isApplicable(const Coupon& coupon, const Cart&) const override {
        return coupon.offer.applicableTo == CouponApplicableTo::PRODUCT;
    }

    bool apply(const Coupon& coupon, const Cart& cart) const override {
        for (Product* product : cart.productList) {
            if (std::find(coupon.offer.applicableProducts.begin(),
                          coupon.offer.applicableProducts.end(),
                          product) == coupon.offer.applicableProducts.end()) {
                return false;
            }
        }
        return true;
    }
};

// ===== REWARD RULES =====

class PercentCouponRewardRule : public ICouponRewardRule {
public:
    bool isApplicable(OfferType offerType) const override {
        return offerType == OfferType::PERCENT_DISCOUNT;
    }

    double apply(const Offer& offer, double amount) const override {
        return amount - (amount * offer.offerPercent / 100);
    }
};

class FlatCouponRewardRule : public ICouponRewardRule {
public:
    bool isApplicable(OfferType offerType) const override {
        return offerType == OfferType::FLAT_DISCOUNT;
    }

    double apply(const Offer& offer, double amount) const override {
        return amount - offer.offerFlat;
    }
};

// ===== COUPON SERVICE =====

class CouponService {
private:
    std::vector<ICouponMatchRule*> matchRules;
    std::vector<ICouponRewardRule*> rewardRules;

public:
    void addMatchRule(ICouponMatchRule* rule) {
        matchRules.push_back(rule);
    }

    void addRewardRule(ICouponRewardRule* rule) {
        rewardRules.push_back(rule);
    }

    double applyCoupon(Cart& cart, const Coupon& coupon) {
        if (!coupon.isActive()) {
            throw std::runtime_error("Invalid or expired coupon.");
        }

        for (auto& rule : matchRules) {
            if (rule->isApplicable(coupon, cart) && !rule->apply(coupon, cart)) {
                throw std::runtime_error("Coupon not applicable.");
            }
        }

        double total = cart.getTotalPrice();
        for (auto& reward : rewardRules) {
            if (reward->isApplicable(coupon.offer.offerType)) {
                return reward->apply(coupon.offer, total);
            }
        }

        return total;
    }
};

// ===== MAIN FUNCTION =====

int main() {
    try {
        // Setup
        User user("u1");
        Seller seller("s1");
        Product p1("p1", 100, &seller);
        Product p2("p2", 200, &seller);

        Cart cart(&user);
        cart.productList.push_back(&p1);
        cart.productList.push_back(&p2);

        TimeRange valid(std::time(nullptr) - 1000, std::time(nullptr) + 10000);
        Offer offer(CouponApplicableTo::SELLER, OfferType::PERCENT_DISCOUNT, 10.0);
        offer.applicableSellers.push_back(&seller);

        Coupon coupon(valid, "SELLER10", offer, &user);

        // Coupon Service Setup
        CouponService service;
        service.addMatchRule(new GlobalCouponMatchRule());
        service.addMatchRule(new SellerCouponMatchRule());
        service.addMatchRule(new ProductCouponMatchRule());
        service.addRewardRule(new PercentCouponRewardRule());
        service.addRewardRule(new FlatCouponRewardRule());

        // Apply
        double discountedPrice = service.applyCoupon(cart, coupon);
        std::cout <<"Original price: "<<cart.getTotalPrice()<< ", Discounted price: " << discountedPrice << std::endl;

    } catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return 0;
}
