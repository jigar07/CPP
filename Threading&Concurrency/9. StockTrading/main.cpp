#include <bits/stdc++.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <list>
#include <unordered_map>
#include <iostream>
#include <sstream>
using namespace std;

enum OrderType {
    BUY,
    SELL,
};

struct Order {
    string id;
    string userId;
    OrderType type;
    int quantity;
    int price;
    string stockName;

    // without following, we cannot delete an order from list. orders.remove will show compilation error.
    bool operator==(const Order& other) const {
        return id == other.id;
    }
};

struct MatchingOrder {
    Order order1;
    Order order2;
    bool isDone;
    MatchingOrder(Order order1, Order order2, bool isDone): order1(order1), order2(order2), isDone(isDone){}
};

struct StockOrder
{
    list<Order> orders;
    mutex mtx;
    condition_variable cv;
};


class StockExecutor {
    mutex& mtx;
    condition_variable& cv;
    list<Order>& orders;
public:
    StockExecutor(list<Order>& orders, mutex& mtx, condition_variable& cv): mtx(mtx), orders(orders), cv(cv) {}
    void run() {
        while(true) {
            MatchingOrder* matchingOrder = NULL;
            unique_lock<mutex> lock(mtx);
            while(true) {
                cout << "Waiting for orders..." << endl;
                while(orders.size() <= 0)
                    cv.wait(lock);

                matchingOrder = findMatchingOrder();

                if(matchingOrder == NULL) {
                    cout << "No matching order found, waiting for more orders..." << endl;
                    cv.wait(lock);
                } else {
                    orders.remove(matchingOrder->order1);
                    orders.remove(matchingOrder->order2);
                    break;
                }
            }
            executeOrder(matchingOrder);
        }
    }

    MatchingOrder* findMatchingOrder() {
        cout << "Finding matching order..." << endl;
        if(orders.size() < 2) {
            cout << "Not enough orders to match." << endl;
            return NULL;
        }
        for(Order order1: orders) {
            for(Order order2: orders) {
                // cout<<order1.id << " vs " << order2.id << endl;
                if(order1.id == order2.id) continue;
                if(order1.type != order2.type &&
                    order1.price == order2.price &&
                    order1.stockName == order2.stockName) {
                        return new MatchingOrder(order1, order2, false);
                    }
            }
        }
        return NULL;
    }

    void executeOrder(MatchingOrder* matchingOrder) {
        cout << "Executing order: " << matchingOrder->order1.id
             << " and " << matchingOrder->order2.id << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
};


class StockExchange {
    unordered_map<string, StockOrder> stockOrderBook;
    mutex mtx;
    // check wht vector is used
    // Using this it will not terminate unexpectedly when main thread exits.
    vector<thread> executorThreads;
public:
    void addExecutor(string stock) {
        unique_lock<mutex> lock(mtx);
        if(stockOrderBook.find(stock) == stockOrderBook.end()) {
            // stockOrderBook[stock] = StockOrder(); shows error because it has mutex and condition_variable. Which makes class non-copyable.
            stockOrderBook.try_emplace(stock);

        }
        executorThreads.push_back(thread(&StockExecutor::run, StockExecutor(stockOrderBook[stock].orders, stockOrderBook[stock].mtx, stockOrderBook[stock].cv)));
        // thread executorThread(&StockExecutor::run, StockExecutor(stockOrderBook[stock].orders, stockOrderBook[stock].mtx, stockOrderBook[stock].cv));
        // // Detach the thread to run independently. When can use detach
        // executorThread.detach();
    }

    void placeOrder(Order order) {
        cout << "Placing order: " << order.id << " for stock: " << order.stockName << endl;
        StockOrder& stockOrder = stockOrderBook[order.stockName];
        unique_lock<mutex> lock(stockOrder.mtx);
        stockOrder.orders.push_back(order);
        cout<< "Order placed: " << order.id << endl;
        stockOrder.cv.notify_all();

    }

    void fetchOrder(string id, string stock) {

    }

    void cancelOrder(Order order) {

    }
    ~StockExchange() {
        for(auto& thread : executorThreads) {
            if(thread.joinable()) {
                thread.join(); // Wait for all threads to finish before exiting
            }
        }
    }
};


int main() {
    StockExchange stockExchange;
    stockExchange.addExecutor("tata");
    stockExchange.addExecutor("reliance");
    stockExchange.addExecutor("reliance");
    int id = 1;

    while(true) {
        string input;
        getline(cin, input);
        cout << "Input: " << input << endl;
        if(input == "exit") {
            cout << "Exiting..." << endl;
            return 0;
        }
        istringstream stream(input);
        string op;
        stream>>op;
        
        if(op == "order") {
            // order 1 tata buy 100 100
            string userId, stockName, orderType;
            int price, quantity;
            stream>>userId>>stockName>>orderType>>price>>quantity;
            stockExchange.placeOrder(Order{to_string(id), userId, orderType == "buy" ? BUY : SELL, quantity, price, stockName});
            id++;
        } else if(op == "cancel") {
            
        }
    }
    return 0;
}