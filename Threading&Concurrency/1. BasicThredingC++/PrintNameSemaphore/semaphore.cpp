#include "semaphore.h"

Semaphore::Semaphore(int count) : count(count) {}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return count > 0; });
    --count;
}

void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mtx);
    ++count;
    cv.notify_one();
}
