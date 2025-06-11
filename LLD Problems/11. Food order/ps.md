System has a tie-up with restaurants, where each restaurant has a menu with all the items & their prices.
Restaurants also have a rating feature which could be given from 1 to 5.
Every restaurant has max # number of orders it can process at any given time. Beyond that, it shouldn’t be assigned any new orders until an ongoing order is completed.
Once an order is ACCEPTED, the restaurant can mark it as COMPLETED when the order is ready. This will free up the processing capacity of the restaurant.
Note:- A restaurant can’t CANCEL an ACCEPTED order.
Order will be auto-assigned to a restaurant based on selection criteria.
Eg: Assign by lowest cost or best rating.
Order will be auto-assigned to a restaurant only if all the items in an order can be fulfilled by a single restaurant. Else the order will not be ACCEPTED.

# Requirements:
- https://enginebogie.com/public/question/low-level-design-food-ordering-system/203
- https://docs.google.com/document/d/1Bmkz9omByHqVvwU45cvkBRSwJAPKw9yaDsRlEnCg_lg/edit?tab=t.0#heading=h.g4ms7p50hwuz

A new restaurant can be onboarded with a menu.
A customer should be able to place an order by giving items, respective quantities & selection criteria.
Restaurants can mark ACCEPTED orders as COMPLETED. Orders once ACCEPTED can’t be CANCELLED by a restaurant.
Order will be auto-assigned to a restaurant based on a selection criteria.
Implement at least one restaurant selection criteria.
A restaurant should be able to update its menu. For simplicity, a restaurant can't delete an item from the menu. Note:- Do not use any database or NoSQL store, use an in-memory store.

# Thought process
- Start with entity classes
- Then core component which is `placeorder`. **OrderingService** created for this. (IOrderRepository to store orders)
- **RestaurantService** to get restaurants (IRestaurantService to store restaurant)
- Now, for placing order OrderInputItem is given. So OrderInputItem is created. Based on the order input filtering is done.
  - `IRestaurantSelectionStrategy` is added to get retaurant based on list of restaurant and OrderInputItem and pick one restaurant among this
    - `SelectionCriteria` created to select restaurant based on criteria
  - `IFilterStrategy` to filter(get list of restaurant) restaurant based on the capacity, supported order etc

# Notes
- Ask recruiter how much time would be given
- Wayfair and few companies asked HLD and LLD in 45 min round of single round only
- Discuss with interviewer and also keep asking should I do this
- Udit’s atlassian interview went well, but still got rejected as he did not write API. Always have active communication with interview regarding what is the expectation
- In ride sharing different folders based on classes. In the interview, Udit writes it in single files but clarifies with the interviewer as well that in production he will write classes in a folder such as model, service etc.
- Folder structure. Following is just a good way to write in different layer, not mandatory. But in prod usually better to have:
    - Controller - User facing layer (API, command, user interface), it actually interact with users
    - Business layer - Managers/services - main business logic
    - Model classes - through which you will interact
- Goal should be atleast start and proceed. As we start with behavior, we might get new classes
  - If getting an idea then go with it, if not then atleast go with any option. At initial stage, we won't be having much idea on which option is based
  - Many times you end up with writing properties, which might be not required later. But keep writing at initial stage and remove later
  - If can go ahead with derived values then use derived values only. Do not start with property during initial phase
- Veg, non-veg, vegan not given in requirement. Good to think about this, but don't increase scope. Can call out to interviewer. In actual company also can discuss with product manager, and if required than only add it
- Don't overthink, keep going ahead
- If single line requirement is given then try to focus on core requirement (Confirm with interviewer that these are the core requirement). ex. for BookMyShow problem, payment is not core requirement. Core requirement is movies and all.
  - Keep focus on core requirement, align with interviewer that this will I do, and this I am not going do.
  - Sometimes if interviewer is not helpful then it will become harder. But try to estimate core requirement and keep the scope smaller

# Design
## Classes:
- MenuItem, Menu, User, Order, Restaurant, Rating
## Properties:
- Restaurant
  - name, id, address, rating, Menu, capacity, concurrentOrdersCapacity, ongoingOrders, completedOrders
  - To keep things simple we can start with single Menu. But in future we can have food menu, drink menu as well
- Menu
  - MenuItem
- RestaurantService
  - added List<Order> here as well. Can add in Restaurant and RestaurantService initially. Then based on which place is simpler we can use there. Also, whether we need Map or List that can also we can decide later
- MenuItem
  - name, quantity, price
  - price, quantity is combine entity. We can create POJO for both
    - So, Price, Quantity, QuantityUnit created (Don't overdo here, our goal is to show interviewer that we have combine entity)
  - isAvailable is derived property. Hence, method `isAvailable` added
- Order
  - OrderStatus, `User placedBy`, `Restaurant servingRestautrant`
  - `finalPrice` methods. 
  - `List<MenuItems>`, or items? Comment right now, if required then will add it in future
  - `List<Price> taxes` also derived entity. Also, if tax is changed then need to modify this. So, better to keep it as derived property
- Rating
  - value
- User
  - id, name
- Don't overthink as we are not going to write userManagement and all
- Try to practise, see if OCP is getting solved. If after that new problem of OCP introduce then solve.
- Implement without going through video or solution. People improved a lot after that

## Behaviors:
- `placeOrder` is most important behavior (Top level behavior we need to implement)
- As placeOrder is more related to OrderingService, can add this and `List<Order>` in `OrderService`. (Note: can keep in `RestaurantService` as well. But if `RestaurantService` become larger than difficult to handle it in sigle class)
- `placeOrder` method
  - Selection Criteria to auto-assigned to a restaurant based on a selection criteria. (Ride service problem for selection/filtering criteria)
  - `OrderInputItem` created
  - `SelectionCriteria` added
  - Following needs to be done:
    - Find restaurants who can serve items.
    - Select one restaurant
    - create order with that restaurant
    - return created order
  - Find restaurants who can serve items.
    - `restaurantService.getAllRestaurants()` called (Added this method in RestaurantService)
    - `restaurantService.getMatchingRestaurants` called (Added this method in RestaurantService)
      - `doYouSupport` added in Restaurant and `Menu`
      - `isMoreThan` added in Quantity
    - `selectedRest` is done in `OrderService`(So, getMatchingRestaurants is not used). Because selectRest is not core functionality of `RestaurantService` (Its personal choice, both are almost equal)
    - Capacity also we need to check (This means whether order is available or not to serve by restaurant)
      - `isAbleToFulfil` added in `Restaurant`
    - **Should we use `List<Order>` in Restaurant or OrderingService?** (This type of things need to be think)
      - Order we can access without Restaurant as well. Order is different entity itself. So, we can't keep it in `Restaurant`
      - We will not be able to map Order to Restaurant in DB schema for SQL DB. For No-SQL, it will create entry very large(Adding/removing order will become difficult)
      - Any related entities should not be there in each other.
      - **So, `Restaurant` having `Order` is not good entity and that's why `List<Order>` removed from `Restaurant` model**
      - So, for this reason `Order` should be in `OrderingService`. Actually not in `OrderServing`, ideally we will be having repository class (Right now in-memory so okay to keep in `OrderingService`, but in future we might get requirement to store it in Redis or DB. So, we can keep in repository folder)
      - **`IOrderRepository` added**
        - For the same reason `IRestaurantRepository` added
        - IRestaurantSelectionStrategy is then used in OrderingService
        - `getAll` method added in `IOrderRepository`
        - `orderRepository.getAll` used in OrderingService
    - `hasCapacity` method added in `Restaurant`
  - Select one restaurant
    - `if` condition based on `selectionCriteria`. OCP is not followed here is another selectionCriteria is added. So, to solve this strategy pattern needs to be used
    - **`IRestaurantSelectionStrategy` added**
      - `doesSupport` added
      - `pickOne` added
    - `List<IRestaurantSelectionStrategy> selectionStrategies` added in OrderingService
      - selectionStrategy and selectionStrategy.pickOne used in OrderingService
  - newOrder created in OrderingService
    - orderRepository.save(newOrder);
  - newOrder is returned
- Improvement:
  - Find restaurants which can serve order:
    - This is not OCP compliant as new filtering criteria can be added
    - To solve this, started with FilterUtils where `capacityAvailableRests` and `supportedRests` are added. But that will also not solve OCP. So, this is not good approach
    - Someone argue that it is just one line of code then why shouldn't we do that instead of following strategy.
      - Because for hotstar, Udit was checking RCA for one of the issue and found that actually one line was removed, reviewer also did not see that. Production broke and payment was broken for 1 hour.
      - So good design is important
      - Regression can happen. So, better to make OCP compliant. (Once in a two year then fine but if it is frequent changes then better to follow OCP)
    - `IFilterStrategy` is added
      - `apply` method
      - `private List<IFilterStrategy> filterStrategies;` added in `OrderingService`
  - `OR` and `AND` combination are solved in Ecommerce filter
  - Threadsafe and concurrency also discussed at the end (Clarify with interview)