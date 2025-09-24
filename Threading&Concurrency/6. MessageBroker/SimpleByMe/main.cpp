#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>

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
    string queueName;
public:
    Consumer(SQS& sqs, string consumerName, string queueName) : sqs(sqs), consumerName(consumerName), queueName(queueName) {}
    void consumeMessage() {
        while(true) {
            string msg = sqs.getMessage();
            if (msg == "") break;
            cout<<queueName<<":"<<consumerName<<" Processing message: " << msg << endl;
            this_thread::sleep_for(chrono::seconds(2));
            cout<<queueName<<":"<<consumerName<<" Processed message: " << msg << endl;
        }
    }
};

class MessageBroker {
    map<string, SQS*> queues;
    public:
    void addConsumer(string queueName, string consumerName) {
        if(queues.find(queueName) == queues.end()) {
            queues[queueName] = new SQS();
        }
        SQS* sqs = queues[queueName];
        // Consumer consumer(*sqs, consumerName, queueName) cannot be used as it will be destroyed after this function
        Consumer* consumer = new Consumer(*sqs, consumerName, queueName);
        thread(&Consumer::consumeMessage, consumer).detach();
        cout<<"Started consumer thread "<<consumerName<<endl;
    }
    void publishMessage(string msg) {
        for(auto& [name, queue] : queues) {
            queue->publishMessage(msg);
        }
    }
};

int main() {
    MessageBroker mb;
    mb.addConsumer("Queue1", "C1");
    mb.addConsumer("Queue1", "C2");
    mb.addConsumer("Queue2", "C3");

    vector<string> msgs = {"msg1", "msg2", "msg3", "msg4", "msg5"};
    for(const auto& msg : msgs) {
        mb.publishMessage(msg);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    this_thread::sleep_for(chrono::seconds(50));
    cout<<"Main thread exiting"<<endl;
    return 0;
}