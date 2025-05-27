#pragma once
#include "IConsumer.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
using namespace std;

class MinusConsumer : public IConsumer {
    atomic<bool> free{true};

public:
    void consume(const CalculationMessage& msg) override {
        free = false;
        cout << this_thread::get_id() << ": Minus started: " << msg << endl;
        this_thread::sleep_for(chrono::seconds(3));
        cout << this_thread::get_id() << ": Minus completed: " <<  msg <<" --> " << (msg.a - msg.b) << endl;
        free = true;
    }

    bool isFree() const override {
        return free.load();
    }
};
