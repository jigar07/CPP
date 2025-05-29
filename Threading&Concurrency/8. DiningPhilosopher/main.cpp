#include <bits/stdc++.h>
using namespace std;

// To make your Fork class more robust, explicitly delete copy and move constructors:
// class Fork {
//     std::mutex mtx;
// public:
//     Fork() = default;
//     Fork(const Fork&) = delete;
//     Fork& operator=(const Fork&) = delete;
//     Fork(Fork&&) = delete;
//     Fork& operator=(Fork&&) = delete;
// };

class Fork {
    // mutex is non-copyable and non-movable.
    // So, any class that contains a mutex (like your Fork class) becomes non-copyable and non-movable unless you explicitly delete or define move/copy operations.
    // So Fork leftFork = forks[index]; will give compiler error. hence, we need to use Fork& leftFork = forks[index];
public:
    timed_mutex mtx;
    Fork() {}
};

class PhiloSopher {
    // why not const vector<Fork>& forks;?
    // Because if vetor is const then this means forks[index] is of type **const Fork&**. Means it is const reference
    // now Fork& leftFork = forks[index];  // ❌ trying to bind non-const reference to const object
    // As unique_lock<mutex> leftLock(leftFork.mtx); we are trying to update mtx, we can not declare leftFork as const
    vector<Fork>& forks;
    const int index;
public:
    PhiloSopher(vector<Fork>& forks, const int& index):
        forks(forks), index(index)
        {}
    void run() {
        Fork& leftFork = forks[index];
        Fork& rightFork = forks[(index + 1) % forks.size()];
        while(true) {
            think();
            // take left lock
            unique_lock<timed_mutex> leftLock(leftFork.mtx);
            if(rightFork.mtx.try_lock_for(chrono::seconds(2))) {
                eat();
                rightFork.mtx.unlock();
            } else {
                cout << "Philosopher " << index << " could not acquire right fork, releasing left and retrying...\n";
            }
        }
    }
    void think() {
        cout<<"Philosopher " << index <<" is thinking" << endl;
        this_thread::sleep_for(chrono::seconds(5));
        cout<<"Philosopher " << index <<" finished thinking" << endl;
    }
    void eat() {
        cout<<"Philosopher " << index <<" is eating" << endl;
        this_thread::sleep_for(chrono::seconds(2));
        cout<<"Philosopher " << index <<" finished eating" << endl;
    }
};

int main() {
    const int numberOfPhilosopher = 5;
    vector<Fork> forks(numberOfPhilosopher);
    vector<PhiloSopher> philoSophers;
    philoSophers.reserve(numberOfPhilosopher);  // Prevent reallocations
    vector<thread> threads;
    for(int i=0; i<numberOfPhilosopher; i++) {
        philoSophers.emplace_back(PhiloSopher(forks, i));
        // The problem is that philoSophers is a std::vector, and vectors can reallocate memory as they grow. If the vector reallocates between emplace_back calls, previous pointers to elements become invalid.
        // So you're potentially passing dangling pointers to the threads, leading to undefined behavior (garbage values, crashes, etc.).
        // Hence, we need to have different for loop for threads.emplace_back
        // We have 2 approaches to solve this:
        // 1. Add philoSophers.reserve(numberOfPhilosopher); To Prevent reallocations
        // 2. Or instead of doing threads.emplace_back here, do it in another for loop outside of this
        threads.emplace_back(&PhiloSopher::run, &philoSophers[i]); // Here threads.push_back will give compiler error
    }
    // for (int i = 0; i < numberOfPhilosopher; ++i) {
    //     threads.emplace_back(&PhiloSopher::run, &philoSophers[i]);
    // }

    for(auto& thread: threads) {
        thread.join();
    }

    return 0;
}