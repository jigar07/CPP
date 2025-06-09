# Problem
* Inventory filter

* You have a inventory of products with you and you want to support filter on top of it.

list of products: [
{
	id: "iphone5"
	name: "iPhone 5",
	price: 1234,
	category: "phone"
},
{
	id: "iphone6"
	name: "iPhone 6",
	price: 2345,
	category: "phone"
}
{
	id: "samsung"
	name: "Samsung ABC",
	price: 837,
	category: "phone"
}
{
	id: "sonybravia-a"
	name: "Sony Bravia A",
	price: 83,
	category: "tv"
}
]

# Thought Process
- Core problem is filter.
- If we think first different filter type can be given. So, if `filter type = "category"`, else if `filter type = price` etc. As we have if else on filter type, we can apply OCP on this and create class for this. This is inherited from **IFilteringCriteria**
  - Now, there is ask of OR/AND filter on different filter types. For that we can simple create class for `AND` and `OR` inherited from IFilteringCriteria itself. But it also have IFilteringCriteria as member variable.
  - There can be nested OR/AND filter also.
  - `IFilterTypeData` also added to store return type of filtered data (Java code)
  - `NotFilteringCriteria` also added to negate the filter. It also has IFilteringCriteria as member variable
- **FilterFactory** and **FilterValue** is also important and core part for problem
# Notes
- Here core problem is filter. No need to implement adding/removing product to inventory if less time.
  - So, Lets assume InventoryManager is given
- If you do not have clear understanding the do not start with strategy pattern and all. If you have clear understanding then go ahead with strategy pattern
- If you get confused then start simple and then improve
- **LLD is not just about watching videos, seeing designs. Its more about creating design, making mistakes and improving design. If you never practise or create design then will never be able to improve**
- Until you don't know the problem, you won't be able to solve it
- Go line by line to find the problem

# Design
- Class InventoryFilter
  - filter method
    - For loop on `allProds` and inside that for loop on `filterProps` then check if value belonging to that key of filterProps available in allProds or not. If yes then add it to filteredList
      - OCP breaks here for new type of filter property
  - `interface IFilteringCriteria` added
  - class `CategoryFilteringCriteria` and `PriceFilteringCriteria` added
  - `private Map<String, IFilteringCriteria> filteringCriteriasMap;` added (Flag based approch for "AND" won't work. matchount would work)
    - We can use `doesSupport` instead of this `Map` if we have use case if we have more Criteria.
    - In that case we will create `List<IFilteringCriteria>` then traverse over this and check `doesSupport` on it
  - `NameFilteringCriteria` added. So, now adding new filtering criteria is east to extend
  - Multiple categories instead of just one.
    - It is easy to support by making change in `doesProductMatch` of `CategoryFilteringCriteria`
    - If required can add "AND"/"OR" based condition in it. But this will be covering later. So, can skip hear 
  - contains/prefix/suffix check instead of equals
    - For this added NameContainsFilteringCriteria, NameEqualsFilteringCriteria. But problem is we are adding too many classes and duplicate code at few places
    - Only single line is changes which `product.getName().contains`
    - To solve this in `NameFilteringCriteria` if condition added which is old way. But again this is OCP problem
    - To solve this `IStringMatchingStrategy` added and `privae Map<String, IStringMatchingStrategy> stringMatchingStrategyMap;` added
  -  No min in price or no max in price.
     -  Right now `if else` should be fine in `PriceFilteringCriteria`
  -  System is fragile. Specially in terms of type safety for fitlerProp values
     -  Fragile here is hardcoded string like "VALUE", "MATCHWAY"
     -  Interface `IFilterTypeData` added for this
        -  `NameFilterTypeData nameFilterData = new ObjecMapper().readValue(filterProps.get("NAME"), NameFilterTypeData.class);` added
        -  This will make contract strong
  -  Either only "OR" is implemented or only "AND" is implemented (How to make combination of this to possible)
     -  If nothing comes to mind then atleast start. Start simple if you are not getting approach
     -  making progress is important
     -  Simplest solution `String orAnd` is passed as an argument in `filter` method and then `if else` condtion in `filter` method
        -  No need to convert this `if else` to strategy as there will not be any other condtion other than `or` and `And`
     -  But it still does not allows combination like `Category OR Name AND price`
        -  This is like expression passing `A OR (B AND C)` (It is recursive approach)
        -  To solve this first we need to get information from `filterProps` map which is passed as argument to `filter` method. (`A OR B AND C` does not tell you anything. Until you put braces like `A OR (B AND C)` you won't understand anything). If braces are not given then you need to convert this into equation with braces using operator pracedence (By maybe taking AND has higher priority than OR)
           -  `A OR B AND C` can be `A OR (B AND C)` or `(A OR B) AND C`
        -  Anyway if it is `(A OR B) AND C` basically means `X AND C`. wheere X is `A OR B`
        -  You can define relationship until you have two operators
        -  It is like:
           -  OR: {
				left: {PRICE: {min: 1, max: 100}}
				right: {CATEGORY: phone}
			}
		-  So it is like new filtering criteria. So, `OrFilteringCriteria` and `AndFilteringCriteria` added
		-  We need to do this part by part. So, recursive way

# Limitations:
* OCP breaks for new type of filter property. - solved - from 1 class change to 0 class change now.
* Multiple categories instead of just one. (Give me a product which has both product and tv. Means give all samsumg product which are tv and phone)
* contains/prefix/suffix check instead of equals
* System is fragile. Specially in terms of type safety for fitlerProp values (Like we are having fix type string, map, double etc)
* Either only "OR" is implemented or only "AND" is implemented (Very important to fix this) (Initial design is "OR" only. To make it "AND" can add matcount and check matchcount matches filterProps.keySet().size(). OR can add boolean flags and check if all boolean flags are true). Change to another needs code change. 
	* Combination is not possible.
* No min in price or no max in price.

* Price having currency filter
	* Looking like new type of filter.

# Code
``` java
// Initial day0 design
/*
 * CATEGORY: phone,
   PRICE: {min: 1, max: 100}
 */
public List<Product> filter(List<Product> allProds, Map<String, Object> filterProps, String orAnd) {
		final List<Product> filteredList = new ArrayList<>();
		for (Product product: allProds) {
			for(String key: filterProps.keySet()) {
				if(key.equals("category")) {
					if(product.getCategory().equals(filterProps.get(key))){
						filteredList.add(product);
						break;
					}
				if(key.equals("PRICE")) {
					Map<String, Double> priceFilter = filterProps.get(key);
					Double minValue = priceFilter.get("min")
					Double maxValue = priceFilter.get("max")
					if(product.getPrice() >= minValue && product.getPrice() <= maxValue){
						filteredList.add(product);
						break;
					}
				}
			}
		}
	}
```
```java

// class InventoryManager {

// 	public static List<Product> allProds() {
// 		return List.of(new Product("iphone5", "iPhone 5", "Samsung ABC", "Sony Bravia A"))
// 	}
// }

class Product {
	String id;
	String name;
	Double amount; // Can use Amount object(POJO) as well which will has Double and Currency as property.
	String category;
}

interface IFilterTypeData {

}


interface IFilteringCriteria {
	boolean doesSupport(String filterName);
	boolean doesProductMatch(Product product, Object filterData);

	List<Product> apply(List<Product> products);
}

class BooleanFilterTypeData implements IFilterTypeData {
	Map<String, Object> left;
	Map<String, Object> right;
}

class OrFilteringCriteria implements IFilteringCriteria {
	private List<IFilteringCriteria> filteringCriteriaList;

	boolean doesSupport(String filterName) {
		return filterName.equals("OR");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		BooleanFilterTypeData orFilterData = new ObjecMapper().readValue(filterProps.get("OR"), BooleanFilterTypeData.class);
		final IFilteringCriteria leftFilteringCriteria = filteringCriteriasMap.get(orFilterData.getLeft().keySet().first());
		final IFilteringCriteria rightFilteringCriteria = filteringCriteriasMap.get(orFilterData.getLeft().keySet().first());

		return leftFilteringCriteria.doesProductMatch(product, orFilterData.getLeft()) || rightFilteringCriteria.doesProductMatch(product, orFilterData.getRight());
	}

	List<Product> apply(List<Product> products) {

	}
}



class AndFilteringCriteria implements IFilteringCriteria {
	private List<IFilteringCriteria> filteringCriteriaList;

	boolean doesSupport(String filterName) {
		return filterName.equals("AND");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		BooleanFilterTypeData orFilterData = new ObjecMapper().readValue(filterProps.get("AND"), BooleanFilterTypeData.class);
		final IFilteringCriteria leftFilteringCriteria = filteringCriteriasMap.get(orFilterData.getLeft().keySet().first());
		final IFilteringCriteria rightFilteringCriteria = filteringCriteriasMap.get(orFilterData.getLeft().keySet().first());

		return leftFilteringCriteria.doesProductMatch(product, orFilterData.getLeft()) && rightFilteringCriteria.doesProductMatch(product, orFilterData.getRight());
	}

	List<Product> apply(List<Product> products) {

	}
}

class NotFilterTypeData implements IFilterTypeData {
	Map<String, Object> operand;
}


class NotFilteringCriteria implements IFilteringCriteria {
	private List<IFilteringCriteria> filteringCriteriaList;

	boolean doesSupport(String filterName) {
		return filterName.equals("NOT");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		NotFilterTypeData filterData = new ObjecMapper().readValue(filterProps.get("NOT"), NotFilterTypeData.class);
		final IFilteringCriteria operandFilteringCriteria = filteringCriteriasMap.get(filterData.getOperand().keySet().first());

		return !operandFilteringCriteria.doesProductMatch(product, filterData.getOperand());
	}

	List<Product> apply(List<Product> products) {

	}
}

class CategoryFilteringCriteria implements IFilteringCriteria {

	boolean doesSupport(String filterName) {
		return filterName.equals("CATEGORY");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		final List<String> categoryFilterData = filterProps.get("CATEGORY");
		return categoryFilterData.stream().anyMatch(inputCat -> inputCat.equals(product.getCategory()));
	}

	List<Product> apply(List<Product> products) {

	}
}


class PriceFilteringCriteria implements IFilteringCriteria {

	boolean doesSupport(String filterName) {
		return filterName.equals("PRICE");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		Map<String, Double> priceFilterData = filterProps.get("PRICE");
		Double minValue = priceFilterData.get("min");
		Double maxValue = priceFilterData.get("max");
		return product.getPrice() >= minValue && product.getPrice() <= maxValue;
	}

	List<Product> apply(List<Product> products) {

	}
}

class NameFilterTypeData implements IFilterTypeData {
	String value;
	String matchWay;
}

class NameFilteringCriteria implements IFilteringCriteria {
	privae Map<String, IStringMatchingStrategy> stringMatchingStrategyMap;

	boolean doesSupport(String filterName) {
		return filterName.equals("NAME");
	}

	boolean doesProductMatch(Product product, Object filterData) {
		NameFilterTypeData nameFilterData = new ObjecMapper().readValue(filterProps.get("NAME"), NameFilterTypeData.class);
		IStringMatchingStrategy stringMS = stringMatchingStrategyMap.get(nameFilterData.matchWay);
		if (stringMS == null) {
			throw new RuntimeException("Unsupported string match");
		}

		return stringMS.isMatching(product.getName(), nameFilterData.value);


		// Old way
		if (matchWay.equals("EQUALS")) {
			return product.getName().equals(nameValue);
		} else if (matchWay.equals("PREFIX")) {
			return product.getName().prefix(nameValue);
		}

		return product.getName().contains(nameValue);
	}

	List<Product> apply(List<Product> products) {

	}
}

interface IStringMatchingStrategy {
	boolean isMatching(String a, String b);
}

class EqualsStringMatchingStrategy implements IStringMatchingStrategy {
	boolean isMatching(String a, String b) {
		return a.equals(b);
	}
}

class PrefixStringMatchingStrategy implements IStringMatchingStrategy {
	boolean isMatching(String a, String b) {
		return a.prefix(b);
	}
}

// 2 + (3 * 5)

// A OR B AND C

// A OR (B AND C)
// (A OR B) AND C AND NOT D
//    X => A OR B

// (price between 1, 100 AND CATEGORY = "phone") OR name = "iphone"


class InventoryFilter {

	/**
		CATEGORY: phone,
		PRICE: {min: 1, max: 100}

		OR: {
			left: {PRICE: {min: 1, max: 100}}
			right: {CATEGORY: phone}
		}

		AND: {
			left: {PRICE: {min: 1, max: 100}}
			right: {CATEGORY: phone}
		}

		NOT: {
			operand: {PRICE: {min: 1, max: 100}}
		}

		OR: {
			left: {AND: {
						left: {PRICE: {min: 1, max: 100}}
						right: {CATEGORY: phone}
					}
			right: {NAME: {"value": "iPhone", "matchWay": "EQUALS"}}
		}
	 */
	private Map<String, IFilteringCriteria> filteringCriteriasMap;

	private List<IFilteringCriteria> filteringCriteriaList;

	public List<Product> filter(List<Product> allProds, Map<String, Object> filterProps, String orAnd) {
		final List<Product> filteredList = new ArrayList<>();
		for (Product product: allProds) {

			boolean doesMatch = false;
			if (orAnd.equals("or")) {
				 doesMatch = filterProps.keySet().stream().anyMatch(key -> {
					final IFilteringCriteria filteringCriteria = filteringCriteriasMap.get(key);
					if (filteringCriteria == null) {
						throw new RuntimeException("No valid filtering criteria found");
					}
					filteringCriteria.doesProductMatch(product, filterProps.get(key))
				});
			}

			if (orAnd.equals("and")) {
				 doesMatch = filterProps.keySet().stream().allMatch(key -> {
					final IFilteringCriteria filteringCriteria = filteringCriteriasMap.get(key);
					if (filteringCriteria == null) {
						throw new RuntimeException("No valid filtering criteria found");
					}
					filteringCriteria.doesProductMatch(product, filterProps.get(key))
				});
			}

			if (doesMatch) {
				filteredList.add(product);
			}
		}
		return filteredList;
	}
}

```

















