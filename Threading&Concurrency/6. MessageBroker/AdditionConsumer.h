#pragma once
#include "IConsumer.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
using namespace std;

class AdditionConsumer : public IConsumer {
    atomic<bool> free{true};

public:
    void consume(const CalculationMessage& msg) override {
        free = false;
        cout << this_thread::get_id() << ": Addition started: " << msg << endl;
        this_thread::sleep_for(chrono::seconds(5));
        cout << this_thread::get_id() << ": Addition completed: " << msg <<" --> " <<(msg.a + msg.b) << endl;
        free = true;
    }

    bool isFree() const override {
        return free.load();
    }
};
