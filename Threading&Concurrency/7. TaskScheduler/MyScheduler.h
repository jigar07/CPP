#pragma once
#include "IConsumer.h"
#include "CalculationMessage.h"
#include "utils.h"
#include "Tasks.h"
#include "ConsumerWorker.h"
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class MyScheduler {
    priority_queue<Tasks> messages;
    vector<thread> threads;
    mutex mtx;
    condition_variable cv;
public:
    void registerConsumer(IConsumer& consumer) {
        cout<<"registering consumer" <<endl;
        threads.emplace_back(ConsumerWorker(consumer, messages, mtx, cv));
    }
    void scheduleAfter(const CalculationMessage& msg, long long millisAfter, long long intervalMillis) {
        scheduleAt(msg, getFutureTimeMillis(millisAfter), intervalMillis);
    }
    void scheduleAt(const CalculationMessage& msg, long long millisAfter, long long intervalMillis) {
        unique_lock<mutex> lock(mtx);
        messages.push({msg, millisAfter, intervalMillis});
        cv.notify_all();
    }
    ~MyScheduler() {
        for(auto& t: threads)
            t.join();
    }
};