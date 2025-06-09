#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <stdexcept>
using namespace std;

// === ENUMS ===

enum class QuantityUnit { KG, G, L, ML };
enum class BeverageType { TEA, COFFEE, HOT_MILK };
enum class OutletStatus { FREE, SERVING };

// === QUANTITY ===

class Quantity {
    QuantityUnit unit;
    int value;

public:
    Quantity(QuantityUnit unit, int value) : unit(unit), value(value) {}

    bool isMoreThan(const Quantity& other) const {
        if (unit != other.unit) throw runtime_error("Unit mismatch");
        return value > other.value;
    }

    void increaseBy(const Quantity& other) {
        if (unit != other.unit) throw runtime_error("Unit mismatch");
        value += other.value;
    }

    void decreaseBy(const Quantity& other) {
        if (unit != other.unit) throw runtime_error("Unit mismatch");
        value -= other.value;
    }

    int getValue() const { return value; }
    QuantityUnit getUnit() const { return unit; }
};

// === INGREDIENT ===

class Ingredient {
    string name;
    Quantity quantity;

public:
    Ingredient() : name(""), quantity(Quantity(QuantityUnit::ML, 0)) {} // Default constructor

    Ingredient(string name, Quantity quantity)
        : name(move(name)), quantity(move(quantity)) {}

    string getName() const { return name; }
    const Quantity& getQuantity() const { return quantity; }
    Quantity& getQuantity() { return quantity; }
};

// === INVENTORY ===

class Inventory {
    vector<Ingredient> ingredients;

public:
    Inventory(vector<Ingredient> ingredients)
        : ingredients(move(ingredients)) {}

    void checkIfSufficient(const Ingredient& required) const {
        auto it = find_if(ingredients.begin(), ingredients.end(),
            [&](const Ingredient& ing) { return ing.getName() == required.getName(); });

        if (it == ingredients.end()) throw runtime_error("Unsupported ingredient: " + required.getName());

        if (required.getQuantity().isMoreThan(it->getQuantity())) {
            throw runtime_error("Insufficient quantity for: " + required.getName());
        }
    }

    void reduceIngredients(const vector<Ingredient>& toReduce) {
        for (const auto& r : toReduce) {
            auto it = find_if(ingredients.begin(), ingredients.end(),
                [&](Ingredient& i) { return i.getName() == r.getName(); });

            if (it == ingredients.end()) throw runtime_error("Missing ingredient in inventory: " + r.getName());

            it->getQuantity().decreaseBy(r.getQuantity());
        }
    }
};

// === BEVERAGE ===

class Beverage {
    BeverageType type;
    string name;
    vector<Ingredient> requiredIngredients;

public:
    Beverage(BeverageType type, string name, vector<Ingredient> requiredIngredients)
        : type(type), name(move(name)), requiredIngredients(move(requiredIngredients)) {}

    BeverageType getType() const { return type; }
    const vector<Ingredient>& getRequiredIngredients() const { return requiredIngredients; }

    void prepare() const {
        cout << "Preparing beverage: " << name << "\n";
    }
};

// === OUTLET ===

class Outlet {
    OutletStatus status = OutletStatus::FREE;

public:
    bool isFree() const { return status == OutletStatus::FREE; }
    void block() { status = OutletStatus::SERVING; }
    void free() { status = OutletStatus::FREE; }

    void vend(const Beverage& bev) {
        cout << "Vending...\n";
        bev.prepare();
        cout << "Beverage ready!\n";
    }
};

// === BATTERY ===

class Battery {
    int percent;

public:
    Battery(int p) : percent(p) {}
    int getPercent() const { return percent; }
};

// === FORWARD DECLARE ===

class CoffeeMachine;

// === VALIDATION INTERFACE ===

class IValidation {
public:
    virtual void validate(BeverageType type, const CoffeeMachine& cm) const = 0;
    virtual ~IValidation() = default;
};

// === VALIDATION IMPLEMENTATIONS ===

class BeverageValidation : public IValidation {
public:
    void validate(BeverageType type, const CoffeeMachine& cm) const override;
};

class SufficientBatteryValidation : public IValidation {
public:
    void validate(BeverageType type, const CoffeeMachine& cm) const override;
};

class FreeOutletValidation : public IValidation {
public:
    void validate(BeverageType type, const CoffeeMachine& cm) const override;
};

// === COFFEE MACHINE ===

class CoffeeMachine {
    vector<Beverage> supportedBeverages;
    Inventory inventory;
    Outlet outlet;
    Battery battery;
    vector<shared_ptr<IValidation>> validations;

public:
    CoffeeMachine(vector<Beverage> beverages,
                  Inventory inventory,
                  Battery battery,
                  vector<shared_ptr<IValidation>> validations)
        : supportedBeverages(move(beverages)),
          inventory(move(inventory)),
          battery(move(battery)),
          validations(move(validations)) {}

    void vendBeverage(BeverageType type, const vector<Ingredient>& additionalIngredients) {
        for (const auto& val : validations) {
            val->validate(type, *this);
        }

        Beverage beverage = getBeverageByType(type);

        // Instead of mergeIngredients, can use decorator pattern. (Double mocha example in head first design pattern)
        auto ingredientsToUse = mergeIngredients(beverage.getRequiredIngredients(), additionalIngredients);

        for (const auto& ing : ingredientsToUse) {
            inventory.checkIfSufficient(ing);
        }

        outlet.block();
        inventory.reduceIngredients(ingredientsToUse);
        outlet.vend(beverage);
        outlet.free();
    }

    const vector<Beverage>& getSupportedBeverages() const { return supportedBeverages; }
    const Inventory& getInventory() const { return inventory; }
    const Battery& getBattery() const { return battery; }
    const Outlet& getOutlet() const { return outlet; }

private:
    Beverage getBeverageByType(BeverageType type) const {
        auto it = find_if(supportedBeverages.begin(), supportedBeverages.end(),
            [&](const Beverage& b) { return b.getType() == type; });

        if (it == supportedBeverages.end()) {
            throw runtime_error("Unsupported beverage type.");
        }

        return *it;
    }

    static vector<Ingredient> mergeIngredients(const vector<Ingredient>& base,
                                                    const vector<Ingredient>& extras) {
        map<string, Ingredient> merged;

        for (const auto& ing : base) {
            merged[ing.getName()] = ing;
        }

        for (const auto& ing : extras) {
            auto it = merged.find(ing.getName());
            if (it != merged.end()) {
                it->second.getQuantity().increaseBy(ing.getQuantity());
            } else {
                merged[ing.getName()] = ing;
            }
        }

        vector<Ingredient> result;
        for (auto& kv : merged) {
            result.push_back(kv.second);
        }

        return result;
    }
};

// === VALIDATION DEFINITIONS ===

void BeverageValidation::validate(BeverageType type, const CoffeeMachine& cm) const {
    const auto& beverages = cm.getSupportedBeverages();
    auto it = find_if(beverages.begin(), beverages.end(),
        [&](const Beverage& b) { return b.getType() == type; });

    if (it == beverages.end()) {
        throw runtime_error("Unsupported beverage.");
    }

    const auto& requiredIngredients = it->getRequiredIngredients();
    for (const auto& ing : requiredIngredients) {
        cm.getInventory().checkIfSufficient(ing);
    }
}


void SufficientBatteryValidation::validate(BeverageType, const CoffeeMachine& cm) const {
    if (cm.getBattery().getPercent() < 50) {
        throw runtime_error("Battery too low.");
    }
}

void FreeOutletValidation::validate(BeverageType, const CoffeeMachine& cm) const {
    if (!cm.getOutlet().isFree()) {
        throw runtime_error("No free outlet.");
    }
}

// === MAIN ===

int main() {
    Ingredient milk("Milk", Quantity(QuantityUnit::ML, 1000));
    Ingredient tea("TeaLeaves", Quantity(QuantityUnit::G, 200));
    Ingredient sugar("Sugar", Quantity(QuantityUnit::G, 150));

    Inventory inventory({milk, tea, sugar});
    Beverage teaBeverage(BeverageType::TEA, "Ginger Tea", {
        Ingredient("Milk", Quantity(QuantityUnit::ML, 200)),
        Ingredient("TeaLeaves", Quantity(QuantityUnit::G, 10)),
        Ingredient("Sugar", Quantity(QuantityUnit::G, 15))
    });

    Battery battery(80);
    vector<shared_ptr<IValidation>> validations = {
        make_shared<BeverageValidation>(),
        make_shared<SufficientBatteryValidation>(),
        make_shared<FreeOutletValidation>()
    };

    CoffeeMachine machine({teaBeverage}, inventory, battery, validations);

    vector<Ingredient> extras = {
        Ingredient("Sugar", Quantity(QuantityUnit::G, 5))  // Extra sugar
    };

    machine.vendBeverage(BeverageType::TEA, extras);

    return 0;
}
