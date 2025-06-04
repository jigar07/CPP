#pragma once
#include <thread>
#include <chrono>
#include "State.h"

void BDRDemocratWorker(State* state) {
    while (true) {
        Person person("", 0);
        {
            std::unique_lock<std::mutex> lock(state->mtx);
            state->cv.wait(lock, [&]() {
                return state->shutdown ||
                        (!state->democrat.empty() &&
                       state->democrat.size() >= state->republic.size() &&
                       state->bathroom.currentRepublicans == 0 &&
                       state->bathroom.currentDemocrats < 3);
            });
            if (state->shutdown) break;
            person = state->democrat.front();
            state->democrat.pop();
            state->bathroom.currentDemocrats++;
            std::cout << person.name << " D using\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(person.millis));

        {
            std::unique_lock<std::mutex> lock(state->mtx);
            state->bathroom.currentDemocrats--;
            std::cout << person.name << " D exited\n";
            state->cv.notify_all();
        }
    }
}
