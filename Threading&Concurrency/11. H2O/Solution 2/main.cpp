#include <bits/stdc++.h>
using namespace std;

struct State {
    int oxygen=0;
    int hydrogen=0;
    bool isWaterPossible() {
        return (oxygen >= 1 &&  hydrogen >= 2);
    }
    void print() {
        cout<<"O: " << oxygen << ", H: " << hydrogen<<endl;
    }
    void releaseIfPossible() {
        if(isWaterPossible())
            releaseWater();
    }
    void releaseWater() {
        cout<<"releasing water: HHO" << endl;
        oxygen -= 1;
        hydrogen -= 2;
    }
};

class H2O {
    State state;
    mutex mtx;
    condition_variable cv;
    bool stopping = false;
public:
    H2O() {}
    void releaseOxygen() {
        unique_lock<mutex> lock(mtx);
        while(state.oxygen == 1 && !stopping) {
            cv.wait(lock);
        }
        state.oxygen += 1;
        cout<<"O: " << state.oxygen <<endl;
        state.releaseIfPossible();
        cv.notify_all();
    }
    void releaseHydroGen() {
        unique_lock<mutex> lock(mtx);
        while(state.hydrogen == 2 && !stopping) {
            cv.wait(lock);
        }
        state.hydrogen += 1;
        cout<<"H: " << state.hydrogen <<endl;
        state.releaseIfPossible();
        cv.notify_all();
    }
    void stop() {
        stopping = true;
        cv.notify_all();
    }
};

int main() {
    H2O h2o;

    vector<thread> threads;
    // for (int i = 0; i < 5; ++i) {
    //     threads.emplace_back([&]() { h2o.releaseHydrogen(); });
    //     threads.emplace_back([&]() { h2o.releaseOxygen(); });
    //     threads.emplace_back([&]() { h2o.releaseHydrogen(); });
    // }

    // for (auto& t : threads) {
    //     t.join();
    // }
    string input;
    while(true) {
        getline(cin, input);
        if(input == "exit") break;
        if(input == "o") threads.emplace_back([&]() { h2o.releaseOxygen(); });
        if(input == "h") threads.emplace_back([&]() { h2o.releaseHydroGen(); });
    }

    h2o.stop();
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }
    return 0;
}