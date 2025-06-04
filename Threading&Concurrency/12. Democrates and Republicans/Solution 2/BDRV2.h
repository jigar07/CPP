#pragma once
#include <string>
#include "State.h"

struct BDRV2 {
    State* state;

    BDRV2(State* s) : state(s) {}

    void democrat(const std::string& name, long millis) {
        std::lock_guard<std::mutex> lock(state->mtx);
        std::cout << name << " D arrived\n";
        state->democrat.push(Person(name, millis));
        state->cv.notify_all();
    }

    void republican(const std::string& name, long millis) {
        std::lock_guard<std::mutex> lock(state->mtx);
        std::cout << name << " R arrived\n";
        state->republic.push(Person(name, millis));
        state->cv.notify_all();
    }
};
