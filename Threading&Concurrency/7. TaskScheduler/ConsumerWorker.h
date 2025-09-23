#pragma once
#include "IConsumer.h"
#include "Tasks.h"
#include "CalculationMessage.h"
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
using namespace std::chrono;

class ConsumerWorker {
    IConsumer& consumer;
    priority_queue<Tasks>& messages;
    mutex& mtx;
    condition_variable& cv;

public:
    ConsumerWorker(IConsumer& consumer, priority_queue<Tasks>& messages, mutex& mtx, condition_variable& cv) :
    consumer(consumer), messages(messages), mtx(mtx), cv(cv) { }
    void operator()() {
        cout << this_thread::get_id() << ": consumer started" << endl;

        // This while loop Keeps thread alive for next tasks
        while (true) {
            Tasks task({{0,0}, 0, 0});

            unique_lock<mutex> lock(mtx);
            // This Waits for the task to become due, and handles wakeups correctly
            // Basically after cv.wait_for wakes up, it again come to this while loop and then process that task if it is available
            // Without while loop and cv.wait_for thread will deque a task prematurely before its scheduled time. So unnecessary resource utilisation. Without this loop, you may consume a task too early, violating your time-based scheduling guarantee.
            while (true) {
                while (messages.empty()) {
                    cout << this_thread::get_id() << ": No element, going to wait" << endl;
                    cv.wait(lock);
                }

                task = messages.top();
                cout << this_thread::get_id() << ": " << task.message << ", picked" << endl;

                long long millisToWait = task.millisEpoch - duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

                cout << this_thread::get_id() << ": " << task.message
                     << ", " << millisToWait << " remaining" << endl;

                if (millisToWait > 0) {
                    cout << this_thread::get_id() << ": " << task.message
                         << ", Going to wait" << endl;

                    cv.wait_for(lock, milliseconds(millisToWait)); // wait for timeout on conditional variable.
                    cout << this_thread::get_id() << ": " << task.message
                         << ", came out of wait" << endl;
                } else {
                    if (task.intervalMillis != -1) {
                        messages.push({task.message, task.millisEpoch + task.intervalMillis, task.intervalMillis});
                        cv.notify_all();
                    }
                    break;
                }
            }

            messages.pop(); // remove the task
            lock.unlock();

            cout << this_thread::get_id() << ": " << task.message
                 << ", consuming" << endl;

            consumer.consume(task.message);
        }
    }
   // following logic may also work where we have only two while loop. But It’s slightly more complex to reason about due to control flow restarting each time. It might incur minor efficiency costs from re-locking and unnecessary condition checks.
    // while (true) {
    //     Tasks task({{0,0}, 0, 0});

    //     unique_lock<mutex> lock(mtx);
    //     bool isTaskAvailable = false;
    //     while (messages.empty()) {
    //         cout << this_thread::get_id() << ": No element, going to wait" << endl;
    //         cv.wait(lock);
    //     }

    //     task = messages.top();
    //     cout << this_thread::get_id() << ": " << task.message << ", picked" << endl;

    //     long long millisToWait = task.millisEpoch - duration_cast<milliseconds>(
    //         system_clock::now().time_since_epoch()).count();

    //     cout << this_thread::get_id() << ": " << task.message
    //             << ", " << millisToWait << " remaining" << endl;

    //     if (millisToWait > 0) {
    //         cout << this_thread::get_id() << ": " << task.message
    //                 << ", Going to wait" << endl;

    //         cv.wait_for(lock, milliseconds(millisToWait));
    //         cout << this_thread::get_id() << ": " << task.message
    //                 << ", came out of wait" << endl;
    //     } else {
    //         if (task.intervalMillis != -1) {
    //             messages.push({task.message, task.millisEpoch + task.intervalMillis, task.intervalMillis});
    //             cv.notify_all();
    //         }
    //         messages.pop(); // remove the task
    //         isTaskAvailable = true;
    //     }

    //     lock.unlock();

    //     if(isTaskAvailable) {
    //         cout << this_thread::get_id() << ": " << task.message
    //             << ", consuming" << endl;

    //         consumer.consume(task.message);
    //     }
    // }

    // Following might also work:
    // while (true) {
    //     Tasks task({{0,0}, 0, 0});

    //     unique_lock<mutex> lock(mtx);
    //     while (messages.empty()) {
    //         cout << this_thread::get_id() << ": No element, going to wait" << endl;
    //         cv.wait(lock);
    //     }

    //     task = messages.top();
    //     cout << this_thread::get_id() << ": " << task.message << ", picked" << endl;

    //     long long millisToWait = task.millisEpoch - duration_cast<milliseconds>(
    //         system_clock::now().time_since_epoch()).count();

    //     while (millisToWait > 0) {
    //         cout << this_thread::get_id() << ": " << task.message
    //             << ", waiting " << millisToWait << " ms" << endl;
    //         cv.wait_for(lock, milliseconds(millisToWait));

    //         millisToWait = task.millisEpoch - duration_cast<milliseconds>(
    //             system_clock::now().time_since_epoch()).count();
    //     }

    //     // Task is due
    //     if (task.intervalMillis != -1) {
    //         messages.push({task.message, task.millisEpoch + task.intervalMillis, task.intervalMillis});
    //         cv.notify_all();
    //     }

    //     messages.pop(); // remove the task
    //     lock.unlock();

    //     cout << this_thread::get_id() << ": " << task.message
    //         << ", consuming" << endl;

    //     consumer.consume(task.message);
    // }

};