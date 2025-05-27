// To enforce enqueue-order fairness, you'd need to write a custom queuing scheduler — which is overkill unless you must simulate fairness. Following code does not ensure enqueue-order fairness.
// I guess with not_full and not_empty it is enqueuing in fairness order. If use while loop then not in faireness
#include <bits/stdc++.h>
#include <mutex>
#include <condition_variable>

using namespace std;

template <typename K>
class threadSafeQueue {
    int maxSize;
    queue<K> q;
    mutex mtx;
    // condition_variable cv;
    condition_variable not_full, not_empty;
public:
    threadSafeQueue(const int& size) : maxSize(size) {}

    void enqueue(K ele) {
        unique_lock<mutex> lock(mtx);
        // while(q.size() >= maxSize) {
        //     cout<< "enqueue: "<< ele <<" going to wait"<<endl;
        //     cv.wait(lock);
        // }
        not_full.wait(lock, [&] { return q.size() < maxSize; });
        cout<< "enqueue: "<< ele <<" and notifying to other"<<endl;
        q.push(ele);
        // cv.notify_all();  // Notify consumers waiting in dequeue
        not_empty.notify_one(); // Should use notify_all only
    }
    K dequeue() {
        unique_lock<mutex> lock(mtx);
        // while(q.empty()) {
        //     cout<< "deque: waiting"<<endl;
        //     cv.wait(lock);
        // }
        not_empty.wait(lock, [&] {return !q.empty();});
        K ele = q.front();
        cout<< "deque: "<<ele <<" and notifying"<<endl;
        q.pop();
        // cv.notify_all();  // Notify producers waiting in enqueue
        not_full.notify_one(); // Should use notify_all only
        return ele;
    }
};

int main() {
    threadSafeQueue<int> q(2);
    string input;
    while (true) {
        getline(cin, input);
        if (input == "exit") break;

        // ostringstream is output only stream
        // stringstream is input and output stream
        // istringstream is Input-only stream.
        // Create an input string stream with the input
        istringstream iss(input);
        string op;
        // iss << op will fail as iss is input stream only
        // it is like cin >> op only.
        iss >> op;

        // Spawn a new thread for each operation. Lambda function is used here
        thread([&q, op, input]() {
            if (op == "en") {
                istringstream in(input);
                string tmp;
                int val;
                in >> tmp >> val;  // skip "en", read the value
                q.enqueue(val);
            } else if (op == "de") {
                q.dequeue();
            }
        }).detach();  // Detach so it runs independently
    }

    return 0;
}
