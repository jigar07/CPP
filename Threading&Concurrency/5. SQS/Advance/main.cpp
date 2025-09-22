#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <string>
#include <atomic>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
using namespace std::chrono;

struct Message {
    string body;
    int priority;
    time_point<steady_clock> enqueueTime;
    milliseconds delay;
    time_point<steady_clock> visibleAt;
    int id;
    time_point<steady_clock> deliveryTime;

    Message() : body(""), priority(0), delay(0ms), id(-1) {
        enqueueTime = deliveryTime = visibleAt = steady_clock::now();
    }

    Message(string b, int p, milliseconds d, int mid)
        : body(move(b)), priority(p), delay(d), id(mid) {
        enqueueTime = steady_clock::now();
        visibleAt = enqueueTime + delay;
    }

    // Priority queue: higher priority = earlier, earlier visibility = earlier
    bool operator<(const Message& other) const {
        if (priority == other.priority)
            return visibleAt > other.visibleAt;
        return priority < other.priority;
    }
};

class SimpleQueueService {
private:
    priority_queue<Message> queue;
    // Know which messages are "in process"
    // Handle visibility timeouts
    // Retry only failed/unacknowledged messages
    // Detect stuck/crashed consumers
    vector<Message> inFlight;
    // Messages are pushed to DLQ if all the retries fails
    map<int, Message> deadLetterQueue;

    mutex mtx;
    condition_variable cv;
    vector<thread> consumers;
    atomic<bool> stop{false};
    int maxRetries = 3;
    int visibilityTimeoutMs = 2000;
    int messageIdCounter = 0;

public:
    void publish(const string& msg, int priority = 0, int delayMs = 0) {
        lock_guard<mutex> lock(mtx);
        queue.push(Message(msg, priority, milliseconds(delayMs), messageIdCounter++));
        cv.notify_all();
    }

    void registerConsumers(const vector<function<bool(const string&)>>& callbacks) {
        for (size_t i = 0; i < callbacks.size(); ++i) {
            consumers.emplace_back([this, i, callback = callbacks[i]]() {
                startConsumerThread(i, callback);
            });
        }
    }

    void shutdown() {
        {
            lock_guard<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& t : consumers)
            if (t.joinable())
                t.join();
    }

    void printDeadLetterQueue() {
        cout << "\n--- Dead Letter Queue ---" << endl;
        for (const auto& [id, msg] : deadLetterQueue) {
            cout << "ID: " << id << ", Body: " << msg.body << endl;
        }
    }

private:
    void startConsumerThread(int id, function<bool(const string&)> callback) {
        while (!stop) {
            unique_lock<mutex> lock(mtx);
            cv.wait_for(lock, milliseconds(100), [this]() {
                return stop || !queue.empty();
            });

            if (stop) break;

            processNextMessage(id, callback);
            handleVisibilityTimeouts();
        }
    }

    void processNextMessage(int id, function<bool(const string&)> callback) {
        if (queue.empty()) return;

        auto now = steady_clock::now();
        Message msg = queue.top();
        if (msg.visibleAt > now) return;

        queue.pop();
        msg.deliveryTime = now;
        inFlight.push_back(msg);

        // Unlock to allow other threads access while processing
        mtx.unlock();
        bool success = callback(msg.body);
        mtx.lock();

        auto it = find_if(inFlight.begin(), inFlight.end(),
                          [&](const Message& m) { return m.id == msg.id; });

        if (success) {
            cout << "[Consumer " << id << "] ACK: " << msg.body << endl;
            inFlight.erase(it);
        } else {
            cout << "[Consumer " << id << "] NACK: " << msg.body << endl;
            if (msg.priority < maxRetries) {
                msg.priority++;
                msg.visibleAt = steady_clock::now() + milliseconds(500);
                queue.push(msg);
            } else {
                cout << "[DLQ] Moving to dead-letter: " << msg.body << endl;
                // deadLetterQueue[msg.id] = msg; or following
                deadLetterQueue.insert({msg.id, msg});
            }
            inFlight.erase(it);
        }
    }

    void handleVisibilityTimeouts() {
        auto now = steady_clock::now();
        for (auto it = inFlight.begin(); it != inFlight.end();) {
            if (now - it->deliveryTime > milliseconds(visibilityTimeoutMs)) {
                cout << "[Visibility Timeout] Re-queueing: " << it->body << endl;
                Message msg = *it;
                msg.visibleAt = now + milliseconds(200);
                queue.push(msg);
                it = inFlight.erase(it);
            } else {
                ++it;
            }
        }
    }
};

// Example usage
int main() {
    SimpleQueueService sqs;

    vector<function<bool(const string&)>> callbacks = {
        [](const string& msg) {
            cout << "[C0] Processing: " << msg << endl;
            return true; // Always succeeds
        },
        [](const string& msg) {
            cout << "[C1] Processing (may fail): " << msg << endl;
            return rand() % 2 == 0; // Succeeds 50% of the time
        },
        [](const string& msg) {
            cout << "[C2] Processing: " << msg << endl;
            return true; // Always succeeds
        }
    };

    sqs.registerConsumers(callbacks);

    for (int i = 0; i < 10; ++i) {
        sqs.publish("Message #" + to_string(i), i % 3, 500);  // Vary priority & delay
    }

    this_thread::sleep_for(chrono::seconds(10));
    sqs.shutdown();
    sqs.printDeadLetterQueue();

    return 0;
}
