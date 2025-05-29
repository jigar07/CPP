#pragma once
#include <chrono>

long long getFutureTimeMillis(long long millisAfter) {
    using namespace std::chrono;
    long long currentTimeMillis = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();

    return currentTimeMillis + millisAfter;
}