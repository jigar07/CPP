#include <iostream>
#include <thread>
#include <chrono>
#include "BDRV2.h"
#include "BDRDemocratWorker.h"
#include "BDRRepublicanWorker.h"
using namespace std;

int main() {
    State state;
    BDRV2 bdrv2(&state);

    // Worker threads
    for (int i = 0; i < 3; ++i) {
        thread(BDRDemocratWorker, &state).detach();
        thread(BDRRepublicanWorker, &state).detach();
    }

    this_thread::sleep_for(chrono::seconds(1));

    thread([&](){ bdrv2.democrat("D1", 5000); }).detach();
    thread([&](){ bdrv2.democrat("D2", 5000); }).detach();
    thread([&](){ bdrv2.republican("R1", 5000); }).detach();
    thread([&](){ bdrv2.democrat("D3", 5000); }).detach();
    thread([&](){ bdrv2.democrat("D4", 5000); }).detach();

    this_thread::sleep_for(chrono::seconds(30)); // Allow threads to finish
    {
        state.shutdown = true;
        state.cv.notify_all(); // Wake up all threads
    }
    return 0;
}
