#include <iostream>
#include <thread>
#include <map>
using namespace std;

struct CalculationMessage {
    int a;
    int b;
    CalculationMessage(int a, int b): a(a), b(b) {}
    friend ostream& operator<<(ostream& os, const CalculationMessage& msg) {
        os << "CalculationMessage(" << msg.a << ", " << msg.b << ")";
        return os;
    }
};

struct State {
    int offset = 0;
    vector<CalculationMessage> messages;
};

class IConsumer {
    public:
    virtual void consume(const CalculationMessage& msg) = 0;
};

class AddConsumer : public IConsumer {
    void consume(const CalculationMessage& msg) override {
        cout << this_thread::get_id() << " " << msg << "Add: " << (msg.a + msg.b) << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
};

class MinusConsumer : public IConsumer {
    void consume(const CalculationMessage& msg) override {
        cout << this_thread::get_id() << " " << msg << "Minus: " << (msg.a - msg.b) << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
};

class ConsumerWorker {
    IConsumer& consumer;
    State& state;
    mutex& mtx;
    condition_variable& cv;
    atomic<bool>& stopped;
    public:
    ConsumerWorker(IConsumer& consumer, State& state, mutex& mtx, condition_variable& cv, atomic<bool>& stopped): consumer(consumer), state(state), mtx(mtx), cv(cv), stopped(stopped) {
    }
    void operator() () {
        while(true) {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [&] { return (state.offset < state.messages.size() || stopped.load() == true); });
            if(stopped.load() == true && state.offset >= state.messages.size()) {
                cout<<"Stopping consumer worker..." << endl;
                break;
            }
            CalculationMessage msg = state.messages[state.offset++];
            lock.unlock();
            consumer.consume(msg);
        }
    }
};

class SQS {
    State state;
    mutex mtx;
    condition_variable cv;
    vector<thread> workers;
    atomic<bool> stopped{false};
    public:
    SQS() {}
    ~SQS() {
        cout<<"Shutting down SQS..." << endl;
        stopped = true;
        cv.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    void registerConsumer(IConsumer& consumer) {
        workers.emplace_back(ConsumerWorker(consumer, state, mtx, cv, stopped));
    }
    void publish(const CalculationMessage& msg) {
        {
            unique_lock<mutex> lock(mtx);
            state.messages.push_back(msg);
        }
        cv.notify_all();
    }
    void resetOffset(size_t newOffset) {
        unique_lock<mutex> lock(mtx);
        if (newOffset < state.messages.size()) {
            state.offset = newOffset;
        } else {
            cerr << "Invalid offset: " << newOffset << endl;
        }
    }
};

class MessageBroker {
    // map<string, SQS*> queues;
    unordered_map<string, shared_ptr<SQS>> queues; // smart pointer to close SQS properly. Other wise if add sleep int main then map<string, SQS*> queues; also works fine.
    public:
    ~MessageBroker() {
        cout<<"Shutting down MessageBroker..." << endl;
    }
    void registerSubscription(string name, IConsumer& consumer) {
        if (queues.find(name) == queues.end()) {
            queues[name] = shared_ptr<SQS>(new SQS());
        }
        queues[name]->registerConsumer(consumer);
    }
    void publish(const CalculationMessage& msg) {
        for (auto& [name, queue] : queues) {
            queue->publish(msg);
        }
    }
    void resetOffset(string name) {
        if(queues.find(name) != queues.end()) {
            queues[name]->resetOffset(0);
        } else {
            cerr << "Subscription not found: " << name << endl;
        }
    }
};

int main() {
    MessageBroker mb;
    AddConsumer addConsumer1, addConsumer2;
    MinusConsumer minusConsumer;
    mb.registerSubscription("AddSubscription", addConsumer1);
    mb.registerSubscription("AddSubscription", addConsumer2);
    mb.registerSubscription("MinusSubscription", minusConsumer);
    mb.publish(CalculationMessage(1, 2));
    mb.publish({10, 20});
    mb.publish({30, 40});
    mb.publish({300, 400});
    // this_thread::sleep_for(chrono::seconds(10));
    return 0;
}


// #include <iostream>
// #include <atomic>
// #include <thread>
// #include <chrono>
// using namespace std;

// struct CalculationMessage {
//     int a;
//     int b;

//     CalculationMessage(int a, int b) : a(a), b(b) {}

//     friend ostream& operator<<(ostream& os, const CalculationMessage& msg) {
//         os << "(" << msg.a << ", " << msg.b << ")";
//         return os;
//     }
// };

// struct SqsState {
//     vector<CalculationMessage> q;
//     size_t offset = 0;
// };

// class IConsumer {
// public:
//     virtual void consume(const CalculationMessage& msg) = 0;
//     virtual bool isFree() const = 0;
//     virtual ~IConsumer() = default;
// };

// class AdditionConsumer : public IConsumer {
//     atomic<bool> free{true};

// public:
//     void consume(const CalculationMessage& msg) override {
//         free = false;
//         cout << this_thread::get_id() << ": Addition started: " << msg << endl;
//         this_thread::sleep_for(chrono::seconds(5));
//         cout << this_thread::get_id() << ": Addition completed: " << msg <<" --> " <<(msg.a + msg.b) << endl;
//         free = true;
//     }

//     bool isFree() const override {
//         return free.load();
//     }
// };

// class MinusConsumer : public IConsumer {
//     atomic<bool> free{true};

// public:
//     void consume(const CalculationMessage& msg) override {
//         free = false;
//         cout << this_thread::get_id() << ": Minus started: " << msg << endl;
//         this_thread::sleep_for(chrono::seconds(3));
//         cout << this_thread::get_id() << ": Minus completed: " <<  msg <<" --> " << (msg.a - msg.b) << endl;
//         free = true;
//     }

//     bool isFree() const override {
//         return free.load();
//     }
// };

// class ConsumerWorker {
//     shared_ptr<SqsState> state;
//     shared_ptr<IConsumer> consumer;
//     mutex& mtx;
//     condition_variable& cv;
//     atomic<bool>& stopped;

// public:
//     ConsumerWorker(shared_ptr<SqsState> state,
//                    shared_ptr<IConsumer> consumer,
//                    mutex& mtx,
//                    condition_variable& cv,
//                    atomic<bool>& stopped)
//         : state(std::move(state)), consumer(std::move(consumer)), mtx(mtx), cv(cv), stopped(std::ref(stopped)) {}

//     void operator()() {
//         while (true) {
//             CalculationMessage msg(0, 0);
//             {
//                 unique_lock<mutex> lock(mtx);
//                 cv.wait(lock, [&](){ return state->offset < state->q.size() || stopped.load() == true; });
//                 if(stopped.load() == true && state->offset >= state->q.size()) {
//                     cout<<"Stopping consumer worker..." << endl;
//                     break;
//                 }
//                 msg = state->q[state->offset++];
//             }
//             consumer->consume(msg);
//         }
//     }
// };

// class SqsQueue {
//     shared_ptr<SqsState> state = make_shared<SqsState>();
//     vector<thread> threads;
//     vector<shared_ptr<IConsumer>> consumers;
//     mutex mtx;
//     condition_variable cv;
//     atomic<bool> stopped{false};

// public:
//     void registerConsumer(shared_ptr<IConsumer> consumer) {
//         cout << "Registering consumer" << endl;
//         consumers.push_back(consumer);
//         threads.emplace_back(ConsumerWorker(state, consumer, mtx, cv, stopped));
//     }

//     void publish(const CalculationMessage& msg) {
//         {
//             lock_guard<mutex> lock(mtx);
//             state->q.push_back(msg);
//         }
//         cv.notify_all();
//     }

//     void resetOffset(size_t newOffset) {
//         lock_guard<mutex> lock(mtx);
//         if (newOffset >= state->q.size()) {
//             throw runtime_error("Invalid offset");
//         }
//         cout << "Resetting offset to: " << newOffset << endl;
//         state->offset = newOffset;
//         cv.notify_all();
//     }

//     ~SqsQueue() {
//         cout<<"Shutting down SqsQueue..." << endl;
//         stopped = true;
//         cv.notify_all();
//         for (auto& t : threads) {
//             if (t.joinable()) t.join();
//         }
//     }
// };

// class MessageBroker {
//     unordered_map<string, shared_ptr<SqsQueue>> queues;

// public:
//     ~MessageBroker() {
//         cout<<"Shutting down MessageBroker..." << endl;
//     }
//     // Register a subscription (queue) and its consumers
//     void registerSubscription(const string& subscriptionName,
//                               const vector<shared_ptr<IConsumer>>& consumers) {
//         if (queues.find(subscriptionName) == queues.end()) {
//             queues[subscriptionName] = make_shared<SqsQueue>();
//         }

//         auto& sqsQueue = queues[subscriptionName];
//         for (const auto& consumer : consumers) {
//             sqsQueue->registerConsumer(consumer);
//         }
//     }

//     // Publish a message to all queues
//     void publish(const CalculationMessage& msg) {
//         for (const auto& [name, queue] : queues) {
//             queue->publish(msg);
//         }
//     }

//     // Reset offset for a specific subscription
//     void resetOffset(const string& subscriptionName, size_t newOffset) {
//         if (queues.find(subscriptionName) != queues.end()) {
//             queues[subscriptionName]->resetOffset(newOffset);
//         } else {
//             cerr << "Subscription not found: " << subscriptionName << endl;
//         }
//     }
// };

// int main() {
//     MessageBroker broker;

//     // Subscription 1: minus consumers
//     broker.registerSubscription("minus-sub", {
//         make_shared<MinusConsumer>(),
//         make_shared<MinusConsumer>()
//     });

//     // Subscription 2: addition consumer
//     broker.registerSubscription("add-sub", {
//         make_shared<AdditionConsumer>()
//     });

//     broker.publish({10, 5});
//     broker.publish({20, 7});
//     broker.publish({8, 3});
//     broker.publish({100, 20});

//     // this_thread::sleep_for(chrono::seconds(20));
//     cout<<"Finished!!!"<<endl;

//     return 0;
// }