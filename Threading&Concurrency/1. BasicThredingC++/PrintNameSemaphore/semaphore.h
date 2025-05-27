#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int count = 0);
    void wait();
    void signal();

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

#endif // SEMAPHORE_H
