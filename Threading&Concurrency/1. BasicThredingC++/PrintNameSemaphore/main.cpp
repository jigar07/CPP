#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "semaphore.h"

using namespace std;

class PrintWord {
public:
    PrintWord(const string& word, Semaphore* mySem, Semaphore* nextSem)
        : word(word), mySem(mySem), nextSem(nextSem) {}

    void run() {
        mySem->wait();
        cout << word << endl;
        this_thread::sleep_for(chrono::milliseconds(300));
        nextSem->signal();
    }

private:
    string word;
    Semaphore* mySem;
    Semaphore* nextSem;
};

int main() {
    Semaphore sem1(1);
    Semaphore sem2(0);
    Semaphore sem3(0);
    Semaphore sem4(0);

    PrintWord task1("Hello", &sem1, &sem2);
    PrintWord task2("I",     &sem2, &sem3);
    PrintWord task3("Am",    &sem3, &sem4);
    PrintWord task4("Jigar", &sem4, &sem1);

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
