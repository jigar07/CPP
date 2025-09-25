#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <map>

using namespace std;

enum OrderType {
    BUY,
    SELL
};

struct Order {
    int orderId;
    int userId;
    string stockName;
    int quantity;
    int price;
    OrderType orderType;
    bool operator==(const Order& order) const {
        return orderId == order.orderId;
    }
};

struct MatchingOrder {
    Order order1;
    Order order2;
};

struct StockOrder {
    list<Order> orders;
    mutex mtx;
    condition_variable cv;
};

class StockExecutor {
    list<Order>& orders;
    mutex& mtx;
    condition_variable& cv;
public:
    StockExecutor(list<Order>& orders, mutex& mtx, condition_variable& cv): orders(orders), mtx(mtx), cv(cv) {}
    void run() {
        while(1) {
            unique_lock<mutex> lock(mtx);
            MatchingOrder* matchingOrder = NULL;
            while(true) {
                matchingOrder = findMatchingOrder();
                if(matchingOrder != NULL) {
                    orders.remove(matchingOrder->order1);
                    orders.remove(matchingOrder->order2);
                    break;
                }
                cv.wait(lock);
            }
            executeOrder(matchingOrder);
        }
    }

    MatchingOrder* findMatchingOrder() {
        if(orders.size() < 2) return NULL;
        for(auto order1: orders) {
            for (auto order2: orders) {
                if(order1.orderId != order2.orderId
                && order1.orderType != order2.orderType
                && order1.stockName == order2.stockName
                && order1.price == order2.price)
                    return new MatchingOrder(order1, order2);
            }
        }
        return NULL;
    }

    void executeOrder(MatchingOrder* matchingOrder) {
        cout<<"Excuting order: "<<matchingOrder->order1.orderId<<", "<<matchingOrder->order2.orderId<<endl;
        this_thread::sleep_for(chrono::seconds(10));
        cout<<"Executed order: "<<matchingOrder->order1.orderId<<", "<<matchingOrder->order2.orderId<<endl;
    }
};

class StockExchange {
    map<string, StockOrder*> stockOrders;
    mutex mtx;
    vector<thread> threads;
public:
    void addStockExecutor(string stockName) {
        unique_lock<mutex> lock(mtx);
        if(stockOrders.find(stockName) == stockOrders.end()) {
            stockOrders[stockName] = new StockOrder();
        }
        StockOrder* stockOrder = stockOrders[stockName];
        threads.emplace_back(&StockExecutor::run, StockExecutor(stockOrder->orders, stockOrder->mtx, stockOrder->cv));
    }

    void placeOrder(Order order) {
        cout<<"Placing order: "<<order.orderId<<": "<<order.stockName<<endl;
        StockOrder* stockOrder = stockOrders[order.stockName];
        unique_lock<mutex> lock(stockOrder->mtx);
        stockOrder->orders.push_back(order);
        cout<<"Placed order: "<<order.orderId<<": "<<order.stockName<<endl;
        stockOrder->cv.notify_all();
    }
};

int main() {
    StockExchange stockExchange;
    stockExchange.addStockExecutor("AMZN");
    stockExchange.addStockExecutor("AAPL");
    stockExchange.addStockExecutor("AMZN");
    while (true) {
        string input;
        // place 1 1 AMZN BUY 100 100
        // place 2 2 AMZN BUY 100 100
        // place 3 3 AAPL BUY 100 100
        // place 4 4 AAPL BUY 100 100
        // place 5 5 AMZN SELL 100 100
        // place 5 5 AAPL SELL 90 100
        // place 5 5 AAPL SELL 100 100

        getline(cin, input);
        if(input == "exit") break;
        istringstream iss(input);
        string operation;
        iss>>operation;
        if(operation == "place") {
            string stockName, orderType;
            int orderId, userId, price, quantity;
            iss >> orderId >> userId >> stockName >> orderType >> price >> quantity;
            Order order = {orderId, userId, stockName, quantity, price, orderType == "BUY" ? OrderType::BUY : OrderType::SELL};
            stockExchange.placeOrder(order);
        } else if(operation == "cancel") {

        }
    }
    return 0;
}