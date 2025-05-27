#include<bits/stdc++.h>
using namespace std;

class SQS {
    queue<string> messages;
    mutex mtx;
    condition_variable cv;
    bool stop = false;
    vector<thread> consumers; // consumers required so that thread object does not get deleted after it created in registerConsumer method. If we dont want this vector then may be we can create object and call object method
public:
    ~SQS() {
        // Stop all threads safely
        {
            lock_guard<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all(); // Wake up all waiting threads
        for (auto& t : consumers) {
            if (t.joinable())
                t.join();
        }
    }

    void consumeMessage(string consumerName, function<void(const string&, const string&)> callBack) {
        while(true) {
            unique_lock<mutex> lock(mtx);
            // predicate version `cv.wait(lock, [this]() { return !messages.empty() || stop; });` is better than while loop
            while(messages.empty() && !stop) {
                cv.wait(lock);
            }
            if (stop && messages.empty()) break;
            string message = messages.front();
            messages.pop();
            lock.unlock();
            callBack(consumerName, message);
        }
    }
    void publishMessage(const string& msg) {
        {
            lock_guard<mutex> lock(mtx);
            messages.push(msg);
        }
        cv.notify_all();
    }
    void registerConsumer(const string& consumerName, function<void(const string&, const string&)> callBack) {
        consumers.emplace_back([=](){
            consumeMessage(consumerName, callBack);
        });
    }
};

void processmessage(const string& consumer, const string& message) {
    cout<<consumer<<" processing message: "<<message<<endl;
    this_thread::sleep_for(chrono::milliseconds(5000));  // simulate processing
    cout<<consumer<<" processed message: "<<message<<endl;
}

int main() {
    SQS sqs;
    sqs.registerConsumer("Consumer-1", processmessage);
    sqs.registerConsumer("Consumer-2", processmessage);
    vector<string> msgs = {"msg1", "msg2", "msg3", "msg4", "msg5"};

    for (const auto& msg : msgs) {
        sqs.publishMessage(msg);
        this_thread::sleep_for(chrono::milliseconds(100)); // Simulate delay
    }

    return 0;
}