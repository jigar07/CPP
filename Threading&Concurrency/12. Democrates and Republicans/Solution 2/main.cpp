#include <iostream>
#include <thread>
#include <chrono>
#include "BDRV2.h"
#include "BDRDemocratWorker.h"
#include "BDRRepublicanWorker.h"

int main() {
    State state;
    BDRV2 bdrv2(&state);

    // Worker threads
    for (int i = 0; i < 3; ++i) {
        std::thread(BDRDemocratWorker, &state).detach();
        std::thread(BDRRepublicanWorker, &state).detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::thread([&](){ bdrv2.democrat("D1", 5000); }).detach();
    std::thread([&](){ bdrv2.democrat("D2", 5000); }).detach();
    std::thread([&](){ bdrv2.republican("R1", 5000); }).detach();
    std::thread([&](){ bdrv2.democrat("D3", 5000); }).detach();
    std::thread([&](){ bdrv2.democrat("D4", 5000); }).detach();

    std::this_thread::sleep_for(std::chrono::seconds(30)); // Allow threads to finish
    {
        state.shutdown = true;
        state.cv.notify_all(); // Wake up all threads
    }
    return 0;
}
