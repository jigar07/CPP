# include <chrono>
# include <thread>
# include <iostream>

using namespace std;

long long getFutureTimeInMillis(long long millisAfter) {
    long long currentTimeMillis = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    return currentTimeMillis + millisAfter;
}

struct CalculationMessage {
    int a;
    int b;

    friend ostream& operator<<(ostream& os, const CalculationMessage& msg) {
        os << "Var1: " << msg.a << ", Var2: " << msg.b;
        return os;
    }
};

struct Task {
    CalculationMessage message;
    long long millisEpoch;
    long long intervalMillis;

    // Comparison operator for priority_queue (min-heap behavior)
    bool operator<(const Task& other) const {
        return millisEpoch > other.millisEpoch;  // earlier times = higher priority
    }
};

class IConsumer {
    public:
    virtual void consume(const CalculationMessage& message) = 0;
    virtual ~IConsumer() = default;
};

class AdditionConsumer : public IConsumer {
    public:
    void consume(const CalculationMessage& message) {
        cout<< this_thread::get_id() <<": CalculationMessage: " << message << ", Addition started: " << message.a + message.b << endl;
        this_thread::sleep_for(chrono::seconds(2));
        cout<< this_thread::get_id() << ": CalculationMessage: " << message << ", Addition finished: " << message.a + message.b << endl;
    }
};

class ConsumerWorker {
    mutex& mtx;
    condition_variable& cv;
    IConsumer& consumer;
    priority_queue<Task>& tasks;
    public:
    ConsumerWorker(mutex& mtx, condition_variable& cv, IConsumer& consumer, priority_queue<Task>& tasks) : mtx(mtx), cv(cv), consumer(consumer), tasks(tasks) {}
    void operator()() {
        cout << this_thread::get_id() << ": consumer started" << endl;
        while(true) {
            Task task;
            while(true) {
                unique_lock<mutex> lock(mtx);
                while(tasks.empty()) {
                    // cout<<this_thread::get_id()<<": waiting for tasks" << endl;
                    cv.wait(lock);
                    // cout<<"task size:" <<tasks.size()<<endl;
                }
                // cout<<"Task: "<<endl;
                task = tasks.top();
                long long millisToWait = task.millisEpoch - chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                if(millisToWait >0) {
                    // cout<<this_thread::get_id()<<": waiting for task for more time: " << task.message << endl;
                    cv.wait_for(lock, chrono::milliseconds(millisToWait));
                } else {
                    tasks.pop();
                    // cout<<this_thread::get_id()<<": consuming task: " << task.message << endl;
                    cout<<"Time: "<<task.millisEpoch<<endl;
                    if(task.intervalMillis != -1) {
                        tasks.push({task.message, task.millisEpoch + task.intervalMillis, task.intervalMillis});
                        cv.notify_all();
                    }
                    break;
                }
            }
            consumer.consume(task.message);
        }
    }
};

class TaskScheduler {
    mutex mtx;
    condition_variable cv;
    priority_queue<Task> tasks;
    vector<thread> workerThreads;
    public:
    void registerConsumer(IConsumer& consumer) {
        workerThreads.emplace_back(ConsumerWorker(mtx, cv, consumer, tasks));
    }
    void scheduleAfter(const CalculationMessage& msg, long long millisAfter, long long intervalInMillis) {
        scheduleAt(msg, getFutureTimeInMillis(millisAfter), intervalInMillis);
    }
    void scheduleAt(const CalculationMessage& msg, long long millisAfter, long long intervalInMillis) {
        unique_lock<mutex> lock(mtx);
        tasks.push({msg, millisAfter, intervalInMillis});
        cv.notify_all();
    }
    ~TaskScheduler() {
        for(auto& workerThread : workerThreads) {
            workerThread.join();
        }
    }
};

int main() {
    TaskScheduler taskScheduler;
    AdditionConsumer a1, a2;
    taskScheduler.registerConsumer(a1);
    taskScheduler.registerConsumer(a2);
    // input:
    // pub 10 20 1000 10000
    while(true) {
        string input;
        getline(cin, input);

        istringstream iss(input);
        string operation;
        iss>>operation;

        if(operation == "exit") break;
        else if(operation == "pub") {
            int a, b;
            long long millisAfter, intervalInMillis;
            iss>>a>>b>>millisAfter>>intervalInMillis;
            CalculationMessage msg{a,b};
            taskScheduler.scheduleAfter(msg, millisAfter, intervalInMillis);
        }
    }
    return 0;
}