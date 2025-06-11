## ThoughtProcess
- VendingMachine main class:
  - It has ingredients (So, list<ingredients>)
  - It has outlets
  - It supports multiple beverages
  - It has method name vend:
    - It will first do validation.
      - For validation (Strategy pattern for this):
        - Check required ingredient for the beverage type is there or not
        - Check additional ingredient are there or not
        - Check vending machine battery
        - Check outlet is free or not
        - As multiple checks are there, better to create class for all different checks
    - After validation block outlet
    - Then, reduce ingredients
    - bev.prepare();
    - outlet.vend(bev);
	- outlet.free();
# Notes
Problem: https://enginebogie.com/public/question/low-level-design-for-coffee-machine/412
- Integration test is asked, means tests needs to be create. So it more of maachine coding than LLD
- Vending machines also similar type of problem. Will be covered at the end.
- It is not given that you should interact with system via API or command line. It is given that you should interact with system via test cases. So, where nothing is given to save your time don't complicate things. Rather directly starts from service class and call from unit or integration tests.
  - **We should focus on core things if APIs or command line input are not given as requirement**
- Once you code behavior, at that time you would know actual properties. So, can add properties or skip propeties during 2nd step. But when we start 3rd step at that time we will be using it actually and come to know whether these properties/classes are required or anything is missing
- As multithreading, consistency not asked in requirement, we are not going to solve this
- OCP is infinite. You solve one problem then can find any other OCP. You have to think where to stop

# Design

- step 1: Classes:
  - class CoffeeMachine (Looks like top like class)
  - class Beverages
  - class Ingredient
  - class Outlet
  - enum BeverageType
- step 2: Start filling properties above classes
  - class CoffeeMachine
    - List<Outlet>, List<Ingredients> inventory, List<BeverageType>, CoffeeMachineStatus
      - `List<Ingredients> inventory` looks like generic things. So, maybe create `class Inventory` for this. Right now will not create. But if require can create in future
      - CoffeMachine has a outlet, outlet can not belong to multiple coffemachine. (Outlet has coffemachine). But coffemachine can be belong to multiple outlet, so List<Outlet> added. Oherwise Outlet class has single CoffeMachine object also fine (In requirement it is given "There would be N (N is an integer) outlet from which beverages can be served." This means there are N outlets in a single CoffeMachine)
        - Outlet here is not shop outlet. It is pipe of CoffeeMachine
  - class Beverage
    - List<Ingredients> requireIngredients, BeverageType
    - Recipe can be added but it is not given in requirement so can skip it at present
  - class Ingredients
    - String name, Quantity quantity
    - class Quantity (Similar to Amount which has Integer and amount type (Rupees/doller), Quantity has Integer and then KG, ML etc)
      - enum QuantityUnit
      - QuantityUnit unit, Integer value
  - class Outlet
    - enum OutletStatus
    - OutletStatus staus
-  step 3: Starts writing methods
   -  CoffeeMachine
      -  vendBeverage method
         -  Why BeverageType as input for this method? Because as I user I will not tell requireIngredients for Beverage and all. As a user I will just tell I need Tea, Coffee etc
         -  // validate
            -  Validate Beverage:
               -  Changed List<BeverageType> to List<Beverage> in CoffeeMachine so that can check whether requireIngredients available or not
               -  for loop on Beverage.requireIngredients
               -  The way of comparing quantity can change. So, `isMoreThan` method added in Quantity
               -  This validation can be completely part of Inventory. Hence, everything moved to `checkIfSufficientIngAvailable` method of Inventory class
               -  Note: Initially it was handled in vendBeverage method only completely(OCP problem). Then `interface IValidation` was added later
            -  Validate Outlet
               -  `isFree` method added in Outlet
         -  // prepare
            -  `block` method added in Outlet
            -  `reduceIngredients` method added in Inventory class (**Even in machine coding round can added ToDo initially. If have time then can implement it later**)
            -  `prepare` method in Beverage class
         -  // vend
            -  `vend` method in Outlet class
            -  `markfree` method in Outlet class
-  Step 4: Issues with above design
   -  **Don't say something goes wrong. Exactly say what may go wrong**. Be very clear
      -  Check each class, method, area of methods to find out issue
      -  Think about if picking one thing changes
      -  Think weird, (Don't think only about if/else)
   -  Another step in prepare, extra ingredients (This could be handled easily)
   -  Another validation -> CoffeeMachine battery.
      -  IValidation added for this
      -  And `List<IValidation> validations;` Inside CoffeeMachine
         -  Another limitation after this:
            - **Right now we are doing AND. What if want to do OR? (Means of any of). For coffee machine we don't need OR. For code perspective it is a limitation but it is not in requirement also**
   -  Way of picking outlet might change.
      -  IOutletPickingStrategy added for this (Though not mentioned in code but it is required)
      -  class `OutletService` created and `List<Outlet>` moved from CoffeeMachine to OutletService
   -  validation and outlet picking are core area for CoffeeMachine
   -  Now, how can I change it to generic VendingMachine or ATM machine from CoffeeMachine
      - Following can change(Note: this is generic):
        - VendingMachine has `Items` instead of `Beverage`. So basically need to make it generic
          - for which CoffeeMachine changed to VendingMachine<I>
          - `List<Beverage>` changed to `List<I>`
        - Steps can change
          - To resolve this `List<IStep>` added. It has steps validation, prepare, vend etc
          - Instead of `List<IStep>` can add seperate object IValidationStep, IPrepareStep and IVendStep as well
   - IReceipeStep, Class Receipe step added. These are generic only
   - At the end for splitwise problem `interface ISplitTypeData` added

# Code

```java
class Inventory {
	private List<Ingredient> ingredients;

	public checkIfSufficientIngAvailable(Ingredient requiredIngredient) {

		Ingredient ingInInv = inventory.stream()
				.filter(ing -> ing.name.equals(requiredIngredient.name))
				.anyMatch()
				.orElse(null)

		if (ingInInv == null) {
			throw new RuntimeException("Unsupported ingredient.");
		}

		if (requiredIngredient.quantity.isMoreThan(ingInInv.quantity) ) {
			throw new RuntimeException("Ingredient not available in the sufficient quantity.");
		}
	}

	public void reduceIngredients(List<Ingredient> ingredientsToReduce) {

		// TODO: Implement this.
	}

}

/* Issues in this design

* Another step in prepare, extra ingredients 
* Another validation -> battery.
* Way of picking outlet might change.
*/

interface IValidation {
	void validate(BeverageType type, CoffeeMachine cm);
}

class BeverageValidation implements IValidation {

	void validate(BeverageType type, CoffeeMachine cm) {
		Beverage bev = cm.getSupportedBeverages().stream()
						.filter(supportedBeverage -> supportedBeverage.getType().equals(type))
						.anyMatch()
						.orElse(null)
		if (bev == null) {
			throw new RuntimeException("Unsupported beverage.");
		}

		Inventory inventory = cm.getInventory();
		for (Ingredient requiredIngredient : bev.requiredIngredients) {
			inventory.checkIfSufficientIngAvailable(requiredIngredient);
		}

	}
}

class SufficientBatteryValidation implements IValidation {
	void validate(BeverageType type, CoffeeMachine cm) {
		if (cm.getBattery().getPercent() < 50) {
			throw new RuntimeException("Running low battery");
		}
	}
}

class FreeOutletValidation implements IValidation {
	private OutletService outletService;
	void validate(BeverageType type, CoffeeMachine cm) {
		if (outletService.checkIfAnyFree(type)) {
			throw new RuntimeException("No free outlet");
		}
	}
}

class OutletService {
	private List<Outlet> outlets; // Pipe which will serve the beverage.
	private IOutletPickingStrategy outletPickingStrategy;

	public boolean checkIfAnyFree(BeverageType type) {
		return outletPickingStrategy.pickOutlet(outlets, type) != null;
	}

	public Outlet getOutlet() {
		return outletPickingStrategy.pickOutlet(outlets, type);
	}
}

class VendingMachine<I> {
	// private List<Outlet> outlets; // Pipe which will serve the beverage.
	private Inventory<I> inventory;
	private List<I> supportedUtens;
	private List<IValidation> validations;
	private IOutletService outletService;
	private List<IStep> steps;

	void vendBeverage(BeverageType type, List<Ingredient> additionalIngredients) {
		// validate - whether beverage is supported and check for presence of required ingredients in inventory. Check for outlet availability.
		validations.forEach(validation -> {
			validation.validate(type, this);
		});

		Outlet outlet = outletService.getOutlet(type);

		// prepare -
		// block an outlet. decrease the ingredient quantity
		outlet.block();
		inventory.reduceIngredients(bev.requiredIngredients);
		bev.prepare();
		// actual prepare - just some dummy method.


		outlet.vend(bev);
		outlet.free();
		// vend - first change outlet status to vending, then vend, then make outlet free again.
	}
}

enum CoffeeMachineStatus {
	FREE,
	IN_PROGRESS,
	PREPARED,
}

enum BeverageType {
	HOT_MILK,
	GINGER_TEA,
	COFFEE,
}

interface IReceipeStep {
	void run();
}

class SwitchOnGasStep implements IReceipeStep {

}

class PourStep implements IReceipeStep {

}

class Receipe {
	String name;
	List<IReceipeStep> recipeSteps;

	void run() {
		recipeSteps.forEach(IReceipeStep::run);
	}
}

class Beverage {
	private List<Ingredient> requiredIngredients;
	private BeverageType type;
	private Receipe receipe;

	public void prepare() {
		receipe.run();
		if (type == TEA) {
			// hApi.switchOnGas();
			// hApi.pourWater();
			// hApi.pourMilk();
			//

		}
		if (type == ICE_TEA) {
			// hApi.pourWater();
			// hApi.pourLemonJuice();
			//

		}
		// do something here.
	}
}

enum QuantityUnit {
	KG,
	G,
	L,
	ML
}

class Quantity {
	private QuantityUnit unit;
	private Integer value;

	boolean isMoreThan(Quantity otherQuantity) {
		if (unit != otherQuantity.unit) {
			throw new RuntimeException("Unsupported units comparison");
		}
		// conversions
		return quantity.value > otherQuantity.quantity.value;
	}
}

class Ingredient {
	private String name;
	private Quantity quantity;
}

enum OutletStatus {
	SERVING,
	FREE
}

class Outlet {
	private OutletStatus status;

	boolean isFree() {
		return status == FREE;
	}

	void block() {
		status = SERVING;
	}

	void free() {
		status = FREE;
	}

	void vend(Beverage bev) {
		// some way it will be implemented
	}
}



```