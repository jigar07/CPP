// To enforce enqueue-order fairness, you'd need to write a custom queuing scheduler — which is overkill unless you must simulate fairness. Following code does not ensure enqueue-order fairness
#include <bits/stdc++.h>
#include <mutex>
#include <condition_variable>

using namespace std;

template <typename K>
class threadSafeQueue {
    int maxSize;
    queue<K> q;
    mutex mtx;
    condition_variable cv;
public:
    threadSafeQueue(const int& size) : maxSize(size) {}

    void enqueue(K ele) {
        unique_lock<mutex> lock(mtx);
        while(q.size() >= maxSize) {
            cout<< "enqueue: "<< ele <<" going to wait"<<endl;
            cv.wait(lock);
        }
        cout<< "enqueue: "<< ele <<" and notifying to other"<<endl;
        q.push(ele);
        cv.notify_all();  // Notify consumers waiting in dequeue
    }
    K dequeue() {
        unique_lock<mutex> lock(mtx);
        while(q.empty()) {
            cout<< "deque: waiting"<<endl;
            cv.wait(lock);
        }
        K ele = q.front();
        cout<< "deque: "<<ele <<" and notifying"<<endl;
        q.pop();
        cv.notify_all();  // Notify producers waiting in enqueue
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

// Check how it is waiting without using while loop in enqueue and dequeue
// #include <iostream>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <queue>
// #include <string>
// #include <sstream>

// using namespace std;

// // Thread-safe bounded blocking queue
// template<typename T>
// class BoundedBlockingQueue {
//     queue<T> q;
//     mutex mtx;
//     condition_variable not_full, not_empty;
//     size_t capacity;

// public:
//     BoundedBlockingQueue(size_t cap) : capacity(cap) {}

//     void enqueue(T val) {
//         unique_lock<mutex> lock(mtx);
//         not_full.wait(lock, [&] { return q.size() < capacity; });
//         q.push(val);
//         cout << "Enqueued: " << val << endl;
//         not_empty.notify_one();
//     }

//     T dequeue() {
//         unique_lock<mutex> lock(mtx);
//         not_empty.wait(lock, [&] { return !q.empty(); });
//         T val = q.front();
//         q.pop();
//         cout << "Dequeued: " << val << endl;
//         not_full.notify_one();
//         return val;
//     }
// };

// // Main controller function
// void run() {
//     BoundedBlockingQueue<int> q(2);  // queue with capacity 2
//     string input;

//     while (true) {
//         getline(cin, input);
//         if (input == "exit") break;

//         istringstream iss(input);
//         string op;
//         iss >> op;

//         // Spawn a new thread for each operation
//         thread([&q, op, input]() {
//             if (op == "en") {
//                 istringstream in(input);
//                 string tmp;
//                 int val;
//                 in >> tmp >> val;  // skip "en", read the value
//                 q.enqueue(val);
//             } else if (op == "de") {
//                 q.dequeue();
//             }
//         }).detach();  // Detach so it runs independently
//     }
// }
