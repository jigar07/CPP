#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;
class SQS {
    queue<string> messages;
    mutex mtx;
    condition_variable cv;
    atomic<bool> stop{false};
    public:
        void shutdown() {
            stop = true;
            cv.notify_all();
        }
        ~SQS () {
            cout<<"Destructor called"<<endl;
            while(!messages.empty()) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        void publishMessage(const string& msg) {
            unique_lock<mutex> lock(mtx);
            messages.push(msg);
            cv.notify_all();
        }

    string getMessage() {
        unique_lock<mutex> lock(mtx);
        // cout<<consumerName<<" "<<stop<<endl;
        while(messages.empty() && !stop) {
            cv.wait(lock);
        }
        if (stop && messages.empty()) {
            cout<<"Stopping thread as destructor called"<<endl;
            return "";
        }
        string msg = messages.front();
        messages.pop();
        return msg;
    }
};

class Consumer {
    SQS& sqs;
    string consumerName;
public:
    Consumer(SQS& sqs, string consumerName) : sqs(sqs), consumerName(consumerName) {}
    void consumeMessage() {
        while(true) {
            string msg = sqs.getMessage();
            if (msg == "") break;
            cout<<consumerName<<" Processing message: " << msg << endl;
            this_thread::sleep_for(chrono::seconds(2));
            cout<<consumerName<<" Processed message: " << msg << endl;
        }
    }
};

int main() {
    SQS sqs;
    Consumer consumer1(sqs, "Consumer1");
    Consumer consumer2(sqs, "Consumer1");
    thread t1(&Consumer::consumeMessage, &consumer1);
    thread t2(&Consumer::consumeMessage, &consumer2);


    vector<string> msgs = {"msg1", "msg2", "msg3", "msg4", "msg5"};
    for(const auto& msg : msgs) {
        sqs.publishMessage(msg);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    sqs.shutdown();
    t1.join(); // Use only if you intend to let threads run independently
    t2.join();
    return 0;
}