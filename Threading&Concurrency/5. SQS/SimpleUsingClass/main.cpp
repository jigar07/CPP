#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
using namespace std;

class SQS;

// Abstract base class for consumers
class Consumer {
protected:
    string name;
    SQS& queue;
    thread worker;
    atomic<bool> running{false};

public:
    Consumer(const string& name, SQS& queue);
    virtual ~Consumer();

    void start();
    void stop();

    // The logic to process a single message
    virtual void process(const string& message) = 0;

private:
    void consumeLoop();
};

// The message queue class
class SQS {
    queue<string> messages;
    mutex mtx;
    condition_variable cv;
    bool stopped = false;
    vector<shared_ptr<Consumer>> consumers;

public:
    ~SQS() {
        stopAll();
    }

    void publishMessage(const string& msg) {
        {
            lock_guard<mutex> lock(mtx);
            messages.push(msg);
        }
        cv.notify_one();
    }

    // Consumers call this to get messages
    optional<string> getNextMessage() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return stopped || !messages.empty(); });

        if (stopped && messages.empty())
            return nullopt;

        string msg = messages.front();
        messages.pop();
        return msg;
    }

    void registerConsumer(shared_ptr<Consumer> consumer) {
        consumers.push_back(consumer);
        consumer->start();
    }

    void stopAll() {
        {
            lock_guard<mutex> lock(mtx);
            stopped = true;
        }
        cv.notify_all();

        for (auto& consumer : consumers) {
            consumer->stop();
        }
    }
};

// Consumer class implementation
Consumer::Consumer(const string& name, SQS& queue)
    : name(name), queue(queue) {}

Consumer::~Consumer() {
    stop();
    if (worker.joinable())
        worker.join();
}

void Consumer::start() {
    running = true;
    worker = thread(&Consumer::consumeLoop, this);
}

void Consumer::stop() {
    running = false;
}

void Consumer::consumeLoop() {
    while (running) {
        auto msg = queue.getNextMessage();
        if (!msg) break;
        process(*msg);
    }
}

// Concrete consumer implementation
class MyConsumer : public Consumer {
public:
    MyConsumer(const string& name, SQS& queue) : Consumer(name, queue) {}

    void process(const string& message) override {
        cout << name << " processing message: " << message << endl;
        this_thread::sleep_for(chrono::milliseconds(2000)); // Simulate work
        cout << name << " processed message: " << message << endl;
    }
};

// Example usage
int main() {
    SQS sqs;

    auto c1 = make_shared<MyConsumer>("Consumer-1", sqs);
    auto c2 = make_shared<MyConsumer>("Consumer-2", sqs);

    sqs.registerConsumer(c1);
    sqs.registerConsumer(c2);

    vector<string> messages = {"msg1", "msg2", "msg3", "msg4", "msg5"};

    for (const auto& msg : messages) {
        sqs.publishMessage(msg);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // Give time for processing
    this_thread::sleep_for(chrono::seconds(10));

    // Destructor of SQS stops everything
    return 0;
}
