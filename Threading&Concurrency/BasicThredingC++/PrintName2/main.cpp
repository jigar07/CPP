// Run method with argument

#include <iostream>
#include <string>
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

class PrintWord {
public:
    PrintWord(State* state, const string& word, const string& nextWord)
        : state(state), word(word), nextWord(nextWord) {}

    // Modified run method to accept an argument
    void run(int times) {
        for (int i = 0; i < times; ++i) {
            unique_lock<mutex> lock(state->mtx);

            while (state->nextWord != word) {
                state->cv.wait(lock);
            }

            cout << word << endl;

            state->nextWord = nextWord;
            state->cv.notify_all();
        }
    }

private:
    State* state;
    string word;
    string nextWord;
};

int main() {
    auto state = new State();
    state->nextWord = "Hello";

    // Create PrintWord tasks
    PrintWord task1(state, "Hello", "I");
    PrintWord task2(state, "I", "Am");
    PrintWord task3(state, "Am", "Jigar");
    PrintWord task4(state, "Jigar", "Hello");

    // Launch threads using &Class::method and object pointer, passing an argument to run
    thread t1(&PrintWord::run, &task1, 5);  // Print "Hello" 5 times
    thread t2(&PrintWord::run, &task2, 5);  // Print "I" 5 times
    thread t3(&PrintWord::run, &task3, 5);  // Print "Am" 5 times
    thread t4(&PrintWord::run, &task4, 5);  // Print "Jigar" 5 times

    // Wait for threads to finish
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    delete state;  // Clean up manually since we used `new`

    return 0;
}


// // Run method without argument
// #include <iostream>
// #include <string>
// #include <mutex>
// #include <condition_variable>
// #include <thread>

// using namespace std;

// class State {
// public:
//     string nextWord;
//     mutex mtx;
//     condition_variable cv;
// };

// class PrintWord {
// public:
//     PrintWord(State* state, const string& word, const string& nextWord)
//         : state(state), word(word), nextWord(nextWord) {}

//     void run() {
//         while(true) {
//             unique_lock<mutex> lock(state->mtx);

//             while (state->nextWord != word) {
//                 state->cv.wait(lock);
//             }

//             cout << word << endl;

//             state->nextWord = nextWord;
//             state->cv.notify_all();
//         }
//     }

// private:
//     State* state;
//     string word;
//     string nextWord;
// };

// int main() {
//     auto state = new State();
//     state->nextWord = "Hello";

//     // Create PrintWord tasks
//     PrintWord task1(state, "Hello", "I");
//     PrintWord task2(state, "I", "Am");
//     PrintWord task3(state, "Am", "Jigar");
//     PrintWord task4(state, "Jigar", "Hello");

//     // Launch threads using &Class::method and object pointer
//     thread t1(&PrintWord::run, &task1);
//     thread t2(&PrintWord::run, &task2);
//     thread t3(&PrintWord::run, &task3);
//     thread t4(&PrintWord::run, &task4);

//     // Wait for threads to finish
//     t1.join();
//     t2.join();
//     t3.join();
//     t4.join();

//     delete state;  // Clean up manually since we used `new`

//     return 0;
// }
