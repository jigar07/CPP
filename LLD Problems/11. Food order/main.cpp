#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <functional>

#include <random>
#include <sstream>

using namespace std;

string generateUUID() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<int> dist(0, 15);
    static uniform_int_distribution<int> dist2(8, 11);

    stringstream ss;
    ss << hex;
    for (int i = 0; i < 8; i++) ss << dist(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dist(gen);
    ss << "-4"; // version 4 UUID
    for (int i = 0; i < 3; i++) ss << dist(gen);
    ss << "-";
    ss << dist2(gen); // variant
    for (int i = 0; i < 3; i++) ss << dist(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dist(gen);
    return ss.str();
}

enum class Currency {
    INR
};

class Price {
public:
    Price(double value, Currency currency) : value(value), currency(currency) {}

    int compare(const Price& other) const {
        if (currency == other.currency) {
            return value < other.value ? -1 : (value > other.value ? 1 : 0);
        }
        throw runtime_error("Currency mismatch");
    }

private:
    double value;
    Currency currency;
};

class Quantity {
public:
    Quantity(double value, string unit) : value(value), unit(unit) {}

    bool isAvailable() const {
        return value > 0;
    }

    bool isMoreThan(const Quantity& other) const {
        if (unit != other.unit) {
            throw runtime_error("Uncomparable Quantities");
        }
        return value > other.value;
    }

private:
    double value;
    string unit;
};

class MenuItem {
public:
    MenuItem(string name, Price price, Quantity quantity)
        : name(move(name)), price(move(price)), quantity(move(quantity)) {}

    const string& getName() const {
        return name;
    }

    const Price& getPrice() const {
        return price;
    }

    const Quantity& getQuantity() const {
        return quantity;
    }

    bool isAvailable() const {
        return quantity.isAvailable();
    }

private:
    string name;
    Price price;
    Quantity quantity;
};

class OrderInputItem {
public:
    OrderInputItem(string name, Quantity quantity)
        : name(move(name)), quantity(move(quantity)) {}

    const string& getName() const {
        return name;
    }

    const Quantity& getQuantity() const {
        return quantity;
    }

private:
    string name;
    Quantity quantity;
};

class Menu {
public:
    void addMenuItem(MenuItem item) {
        menuItems.push_back(move(item));
    }

    bool doYouSupport(const vector<OrderInputItem>& orderInputItems) const {
        return all_of(orderInputItems.begin(), orderInputItems.end(), [this](const OrderInputItem& oi) {
            return any_of(menuItems.begin(), menuItems.end(), [&oi](const MenuItem& menuItem) {
                return menuItem.getName() == oi.getName() && menuItem.getQuantity().isMoreThan(oi.getQuantity());
            });
        });
    }

private:
    vector<MenuItem> menuItems;
};

class Order {
public:
    Order(string id, string status, string placedById, string restaurantId)
        : id(move(id)), status(move(status)), placedById(move(placedById)), restaurantId(move(restaurantId)) {}

    const string& getId() const {
        return id;
    }

    const string& getStatus() const {
        return status;
    }

    const string& getPlacedById() const {
        return placedById;
    }

    const string& getRestaurantId() const {
        return restaurantId;
    }

private:
    string id;
    string status;
    string placedById;
    string restaurantId;
};

class User {
public:
    User(string id, string name) : id(move(id)), name(move(name)) {}

    const string& getId() const {
        return id;
    }

    const string& getName() const {
        return name;
    }

private:
    string id;
    string name;
};

class Restaurant {
public:
    Restaurant(string id, string name, Menu menu, int concurrentOrdersCapacity)
        : id(move(id)), name(move(name)), menu(move(menu)), concurrentOrdersCapacity(concurrentOrdersCapacity) {}

    const string& getId() const {
        return id;
    }

    const string& getName() const {
        return name;
    }

    const Menu& getMenu() const {
        return menu;
    }

    int getConcurrentOrdersCapacity() const {
        return concurrentOrdersCapacity;
    }

    bool doYouSupport(const vector<OrderInputItem>& orderInputItems) const {
        return menu.doYouSupport(orderInputItems);
    }

private:
    string id;
    string name;
    Menu menu;
    int concurrentOrdersCapacity;
};

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual vector<Order> getAll() const = 0;
    virtual void save(const Order& newOrder) = 0;
};

class IRestaurantRepository {
public:
    virtual ~IRestaurantRepository() = default;
    virtual vector<Restaurant> getAll() const = 0;
};

class InMemoryOrderRepository : public IOrderRepository {
public:
    vector<Order> getAll() const override {
        return orders;
    }

    void save(const Order& newOrder) override {
        orders.push_back(newOrder);
    }

private:
    vector<Order> orders;
};

class InMemoryRestaurantRepository : public IRestaurantRepository {
public:
    vector<Restaurant> getAll() const override {
        return restaurants;
    }

    void addRestaurant(Restaurant restaurant) {
        restaurants.push_back(move(restaurant));
    }

private:
    vector<Restaurant> restaurants;
};

// --- Interfaces ---

class IFilterStrategy {
public:
    virtual ~IFilterStrategy() = default;
    virtual vector<Restaurant> apply(const vector<Restaurant>& restaurants, const vector<OrderInputItem>& orderInputItems, const vector<Order>& allOrders) const = 0;
};

class IRestaurantSelectionStrategy {
public:
    virtual ~IRestaurantSelectionStrategy() = default;
    virtual bool doesSupport(const string& selectionCriteria) const = 0;
    virtual Restaurant pickOne(const vector<Restaurant>& restaurants, const vector<OrderInputItem>& orderInputItems) const = 0;
};

// --- Filter Strategies ---

class SupportedOrderInputsRestFilterStrategy : public IFilterStrategy {
public:
    vector<Restaurant> apply(const vector<Restaurant>& restaurants, const vector<OrderInputItem>& orderInputItems, const vector<Order>&) const override {
        vector<Restaurant> filtered;
        copy_if(restaurants.begin(), restaurants.end(), back_inserter(filtered), [&](const Restaurant& rest) {
            return rest.doYouSupport(orderInputItems);
        });
        return filtered;
    }
};

class AvailableCapacityRestaurantFilterStrategy : public IFilterStrategy {
public:
    vector<Restaurant> apply(const vector<Restaurant>& restaurants, const vector<OrderInputItem>&, const vector<Order>& allOrders) const override {
        vector<Restaurant> filtered;
        for (const auto& rest : restaurants) {
            int inProgressOrders = count_if(allOrders.begin(), allOrders.end(), [&](const Order& order) {
                return order.getStatus() == "ACCEPTED" && order.getRestaurantId() == rest.getId();
            });
            if (rest.getConcurrentOrdersCapacity() > inProgressOrders) {
                filtered.push_back(rest);
            }
        }
        return filtered;
    }
};

// --- Selection Strategies ---

class LowestCostRestaurantSelectionStrategy : public IRestaurantSelectionStrategy {
public:
    bool doesSupport(const string& selectionCriteria) const override {
        return selectionCriteria == "LOWEST_COST";
    }

    Restaurant pickOne(const vector<Restaurant>& restaurants, const vector<OrderInputItem>&) const override {
        if (restaurants.empty()) throw runtime_error("No restaurants to select from.");

        // Stub logic — in a real case, you'd compute actual prices
        return *min_element(restaurants.begin(), restaurants.end(), [](const Restaurant& r1, const Restaurant& r2) {
            return r1.getId() < r2.getId();  // Placeholder logic
        });
    }
};


class OrderingService {
public:
    OrderingService(
        shared_ptr<IOrderRepository> orderRepository,
        shared_ptr<IRestaurantRepository> restaurantRepository,
        vector<shared_ptr<IFilterStrategy>> filterStrategies,
        vector<shared_ptr<IRestaurantSelectionStrategy>> selectionStrategies)
        : orderRepository(move(orderRepository)),
          restaurantRepository(move(restaurantRepository)),
          filterStrategies(move(filterStrategies)),
          selectionStrategies(move(selectionStrategies)) {}

    Order placeOrder(const vector<OrderInputItem>& orderInputItems, const User& user, const string& selectionCriteria) {
        auto restaurants = restaurantRepository->getAll();
        auto allOrders = orderRepository->getAll();

        for (const auto& strategy : filterStrategies) {
            restaurants = strategy->apply(restaurants, orderInputItems, allOrders);
        }

        auto it = find_if(selectionStrategies.begin(), selectionStrategies.end(), [&](const shared_ptr<IRestaurantSelectionStrategy>& strategy) {
            return strategy->doesSupport(selectionCriteria);
        });

        if (it == selectionStrategies.end()) {
            throw runtime_error("No selection strategy supports the criteria.");
        }

        Restaurant selected = (*it)->pickOne(restaurants, orderInputItems);

        Order order(generateUUID(), "ACCEPTED", user.getId(), selected.getId());
        orderRepository->save(order);
        return order;
    }

private:
    shared_ptr<IOrderRepository> orderRepository;
    shared_ptr<IRestaurantRepository> restaurantRepository;
    vector<shared_ptr<IFilterStrategy>> filterStrategies;
    vector<shared_ptr<IRestaurantSelectionStrategy>> selectionStrategies;
};

int main() {
    // Create repository and sample data
    auto restaurantRepo = make_shared<InMemoryRestaurantRepository>();
    auto orderRepo = make_shared<InMemoryOrderRepository>();

    Price price1(100.0, Currency::INR);
    Quantity quantity1(10, "NUMS");
    MenuItem menuItem1("Pizza", price1, quantity1);
    Menu menu;
    menu.addMenuItem(menuItem1);
    Restaurant rest("1", "Pizza Hut", menu, 5);
    restaurantRepo->addRestaurant(rest);

    // Add filters and selection strategy
    vector<shared_ptr<IFilterStrategy>> filters = {
        make_shared<SupportedOrderInputsRestFilterStrategy>(),
        make_shared<AvailableCapacityRestaurantFilterStrategy>()
    };

    vector<shared_ptr<IRestaurantSelectionStrategy>> selectors = {
        make_shared<LowestCostRestaurantSelectionStrategy>()
    };

    OrderingService orderingService(orderRepo, restaurantRepo, filters, selectors);
    User user("u123", "Alice");

    vector<OrderInputItem> items = {
        OrderInputItem("Pizza", Quantity(2, "NUMS"))
    };

    try {
        Order order = orderingService.placeOrder(items, user, "LOWEST_COST");
        cout << "Order placed successfully. ID: " << order.getId() << "\n";
    } catch (const exception& e) {
        cerr << "Failed to place order: " << e.what() << "\n";
    }

    return 0;
}
