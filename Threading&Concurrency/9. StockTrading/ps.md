Codewriting
Design and implement an efficient in-memory trading system similar to a stock exchange, where registered users can place, execute and cancel trades. The system should demonstrate synchronization and concurrency in a multi-threaded environment.
Your system should be able to perform the following items:

Functional Requirements:
Your system should support the following functionalities:
* A registered user can place, modify, and cancel his orders.
* A user should be able to query the status of his order
* [IMPORTANT] - The system should be able to execute trades based on matching buy and sell orders. A trade is executed when the buy and sell price of two different orders match/are equal. If multiple eligible orders can be matched with the same price, match the oldest orders first.
* [IMPORTANT] - Concurrent order placement, modification, cancellation, and execution should be handled appropriately.
* The system should maintain an order book per symbol, which holds all the current unexecuted orders.
  Your system should store at least the following mentioned details.
* User details
    * User ID
    * User Name
    * Phone Number
    * Email Id
* Orders
    * Order ID
    * User ID
    * OrderType (Buy/Sell)
    * Stock Symbol (eg: RELIANCE, WIPRO etc.)
    * Quantity
    * Price
    * Order Accepted Timestamp
    * Status (ACCEPTED, REJECTED, CANCELED)
* Trades (Trade means order which is executed)
    * Trade ID
    * Trade Type (Buy/Sell)
    * Buyer Order Id
    * Seller Order Id
    * Stock Symbol
    * Quantity
    * Price
    * Trade Timestamp

Additional functionality, but not compulsory [implement if time permits]:
* Implement order expiry. A order should be automatically canceled if that order is not executed within a specific time.
  * Use scheduler to cancel/remove order
  * Use lazy scheduler to cancel/remove order while matching order
  * Use global expiry to cancel/remove all order when market closes

* Expectation:
* Your code should be executable (at worst partial running would work) & should be clean.
* Your code should be adequately refactored, and exceptions should be gracefully handled.
* Your code should store all the attributes explained under the "Stores" section.
* Your code should cover all the functionality in the "Supports" section.
* If you get extra time, you can code for " Additional functionality, but not compulsory." This will get you extra credit.
  Guidelines:
* You don't have to build the user registration part. Consider some dummy users registered and use those in the entire system.
* You should use the in-memory data structure of your preferred language to store the data but have the right abstractions so that other persistent stores can be plugged in.
  Evaluation criteria:
* Executable code.
* Code readability and testability
* Refactored code
* Abstraction
* Object-Oriented concepts.
* Language proficiency.
* [execution time limit] 3 seconds (java)
* [memory limit] 1 GB


* During interview try to achieve depth rather than breadth. For depth core functionality must be prioritised. For example for stock exchange place order must be prioritised than order status.
* For multithreading code always think what can be done asynchronously
* Each stock can be executed concurrency as each stock is independent entity. So, can minimize blast redious using separate thread for separate stock. (In that case lock might not be required)


### Thought process
- Think like for each stock separate orderBook will be there for its order
- Then for each stock there will be one excutor to match the order
  - Then for single stock multiple executor could be there which matches the order.
  - Now, we need to maintain concurrency for each stock such that same order is not executed by multiple executor consider we have multiple threads for same stock.
  - That means orderbook of single stock will be shared between multiple threads