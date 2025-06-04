#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "BathroomV2.h"
#include "Person.h"

struct State {
    BathroomV2 bathroom;
    std::queue<Person> democrat;
    std::queue<Person> republic;
    std::mutex mtx;
    std::condition_variable cv;
    bool shutdown = false;
};
