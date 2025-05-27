#pragma once
#include "CalculationMessage.h"
#include "IConsumer.h"
#include "ConsumerWorker.h"
#include "SqsState.h"
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>
using namespace std;

class SqsQueue {
    shared_ptr<SqsState> state = make_shared<SqsState>();
    vector<thread> threads;
    vector<shared_ptr<IConsumer>> consumers;
    mutex mtx;
    condition_variable cv;

public:
    void registerConsumer(shared_ptr<IConsumer> consumer) {
        cout << "Registering consumer" << endl;
        consumers.push_back(consumer);
        threads.emplace_back(ConsumerWorker(state, consumer, mtx, cv));
    }

    void publish(const CalculationMessage& msg) {
        {
            lock_guard<mutex> lock(mtx);
            state->q.push_back(msg);
        }
        cv.notify_all();
    }

    void resetOffset(size_t newOffset) {
        lock_guard<mutex> lock(mtx);
        if (newOffset >= state->q.size()) {
            throw runtime_error("Invalid offset");
        }
        cout << "Resetting offset to: " << newOffset << endl;
        state->offset = newOffset;
        cv.notify_all();
    }

    ~SqsQueue() {
        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    }
};
