#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;

class State {
public:
    string nextWord;
    mutex mtx;
    condition_variable cv;
};

// PrintWord task, similar to Java Runnable
class PrintWord {
public:
    // PrintWord(shared_ptr<State> state, const string& word, const string& nextWord)
    PrintWord(State* state, const string& word, const string& nextWord)
        : state(move(state)), word(word), nextWord(nextWord) {}

    void operator()() {
        unique_lock<mutex> lock(state->mtx);

        // Classic while loop instead of lambda
        while (state->nextWord != word) {
            state->cv.wait(lock);
        }

        cout << word << endl;

        state->nextWord = nextWord;
        state->cv.notify_all();
    }

private:
    State* state;
    string word;
    string nextWord;
};

int main() {
    // auto state = make_shared<State>();
    auto state = new State();
    state->nextWord = "Hello";

    // Create PrintWord tasks
    PrintWord task1(state, "Hello", "I");
    PrintWord task2(state, "I", "Am");
    PrintWord task3(state, "Am", "Jigar");
    PrintWord task4(state, "Jigar", "DONE");
    // Launch threads
    thread t1(task1);
    thread t2(task2);
    thread t3(task3);
    thread t4(task4);

    // Wait for threads to finish
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}
