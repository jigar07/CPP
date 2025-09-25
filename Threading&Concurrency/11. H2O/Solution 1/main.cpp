#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

struct State {
    int oxygen;
    int hydrogen;
    State() {
        oxygen=0;
        hydrogen=0;
    }
    bool isWaterPossible() {
        return (oxygen >= 1 &&  hydrogen >= 2);
    }
    void print() {
        cout<<"O: " << oxygen << ", H: " << hydrogen<<endl;
    }
    void releaseWater() {
        cout<<"releasing water: HHO" << endl;
        oxygen -= 1;
        hydrogen -= 2;
    }
};

class WaterMoleculeWorker {
    State& state;
    mutex& mtx;
    condition_variable& cv;
    atomic<bool>& stopFlag;
public:
    WaterMoleculeWorker(State& state, mutex& mtx, condition_variable& cv, atomic<bool>& stopFlag):
    state(state), mtx(mtx), cv(cv), stopFlag(stopFlag) {}
    void run() {
        while(true) {
            unique_lock<mutex> lock(mtx);
            while(!state.isWaterPossible() && !stopFlag.load()) {
                cv.wait(lock);
            }
            if (stopFlag.load()) break;
            state.releaseWater();
            lock.unlock();
            this_thread::sleep_for(chrono::seconds(5));
            state.print();
        }
    }
};

class H2O {
    State state;
    mutex mtx;
    condition_variable cv;
    vector<thread> workerThreads;
    atomic<bool> stopFlag; // exiting when exit is entered
public:
    H2O() {}
    void startworkerThread() {
        workerThreads.emplace_back(&WaterMoleculeWorker::run, WaterMoleculeWorker(state, mtx, cv, stopFlag));
    }
    void releaseOxygen() {
        unique_lock<mutex> lock(mtx);
        state.oxygen += 1;
        state.print();
        cv.notify_all();
    }
    void releaseHydroGen() {
        unique_lock<mutex> lock(mtx);
        state.hydrogen += 1;
        state.print();
        cv.notify_all();
    }
    void shutdown() {
        {
            unique_lock<mutex> lock(mtx);
            stopFlag = true;
        }
        cv.notify_all();
        for(auto& workerThread : workerThreads) {
            if (workerThread.joinable()) {
                workerThread.join();  // Proper cleanup
            }
        }
    }

    ~H2O() {
        shutdown(); // Ensures clean termination
    }
};

int main() {
    H2O h2o;
    h2o.startworkerThread();
    h2o.startworkerThread();
    // h2o.releaseOxygen();
    // h2o.releaseHydroGen();
    // h2o.releaseHydroGen();
    // this_thread::sleep_for(chrono::seconds(1));
    string input;
    while(true) {
        getline(cin, input);
        if(input == "exit") break;
        if(input == "O") h2o.releaseOxygen();
        if(input == "H") h2o.releaseHydroGen();
    }
    return 0;
}