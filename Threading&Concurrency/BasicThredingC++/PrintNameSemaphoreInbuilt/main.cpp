#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

class PrintWord {
public:
    // Change the constructor to accept references instead of by value
    PrintWord(const string& word, counting_semaphore<>& mySem, counting_semaphore<>& nextSem)
        : word(word), mySem(mySem), nextSem(nextSem) {}

    void run() {
        mySem.acquire();  // Wait operation (decrement)
        cout << word << endl;
        this_thread::sleep_for(chrono::milliseconds(300));
        nextSem.release();  // Signal operation (increment)
    }

private:
    string word;

    
    // counting_semaphore is non-copyable. You should pass the semaphores by reference to the constructor (instead of copying them). This will allow the semaphores to be shared between threads without trying to copy them.
    counting_semaphore<>& mySem;  // Reference to semaphore
    counting_semaphore<>& nextSem; // Reference to semaphore
};

int main() {
    // Use inbuilt semaphore instead of custom semaphore
    counting_semaphore<> sem1(1);  // Initially 1 (allows task1 to start)
    counting_semaphore<> sem2(0);  // Initially 0 (task2 waits for task1)
    counting_semaphore<> sem3(0);  // Initially 0 (task3 waits for task2)
    counting_semaphore<> sem4(0);  // Initially 0 (task4 waits for task3)

    PrintWord task1("Hello", sem1, sem2);
    PrintWord task2("I",     sem2, sem3);
    PrintWord task3("Am",    sem3, sem4);
    PrintWord task4("Jigar", sem4, sem1);

    thread t1(&PrintWord::run, &task1);
    thread t2(&PrintWord::run, &task2);
    thread t3(&PrintWord::run, &task3);
    thread t4(&PrintWord::run, &task4);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}
