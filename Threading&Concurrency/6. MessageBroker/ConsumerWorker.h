#pragma once
#include "IConsumer.h"
#include "SqsState.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
using namespace std;

class ConsumerWorker {
    shared_ptr<SqsState> state;
    shared_ptr<IConsumer> consumer;
    mutex& mtx;
    condition_variable& cv;

public:
    ConsumerWorker(shared_ptr<SqsState> state,
                   shared_ptr<IConsumer> consumer,
                   mutex& mtx,
                   condition_variable& cv)
        : state(move(state)), consumer(move(consumer)), mtx(mtx), cv(cv) {}

    void operator()() {
        while (true) {
            CalculationMessage msg(0, 0);
            {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [&]() { return state->offset < state->q.size(); });
                msg = state->q[state->offset++];
            }
            consumer->consume(msg);
        }
    }
};
