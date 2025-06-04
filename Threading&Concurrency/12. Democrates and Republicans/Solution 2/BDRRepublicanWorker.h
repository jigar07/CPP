#pragma once
#include <thread>
#include <chrono>
#include "State.h"

void BDRRepublicanWorker(State* state) {
    while (true) {
        Person person("", 0);
        {
            std::unique_lock<std::mutex> lock(state->mtx);
            state->cv.wait(lock, [&]() {
                return state->shutdown ||
                        (!state->republic.empty() &&
                       state->republic.size() >= state->democrat.size() &&
                       state->bathroom.currentDemocrats == 0 &&
                       state->bathroom.currentRepublicans < 3);
            });
            if (state->shutdown) break;
            person = state->republic.front();
            state->republic.pop();
            state->bathroom.currentRepublicans++;
            std::cout << person.name << " R using\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(person.millis));

        {
            std::unique_lock<std::mutex> lock(state->mtx);
            state->bathroom.currentRepublicans--;
            std::cout << person.name << " R exited\n";
            state->cv.notify_all();
        }
    }
}
