// With threading
#include<mutex>
#include<condition_variable>
#include<thread>
#include<iostream>

using namespace std;

struct State {
    string word;
    mutex mtx;
    condition_variable cv;
    State(const string& word): word(word) {}
};

class PrintWord {
    State& state; // Make sure State& instead of State. Otherwise it will create new copy locally
    string word;
    string nextWord;
public:
    PrintWord(State& state, const string& word, const string& nextWord)
    :state(state), word(word), nextWord(nextWord) {}
    void operator()() {
        // Automatically unlock the mutex when it goes out of scope. If want custom unlock then call lock.unlock()
        // Instead if unique_lock can use state.mtx.lock() and state.mtx.unlock(). But with this have to manage unlock as weill
        unique_lock<mutex> lock(state.mtx);
        // In this example, the if (state.word == word) check only happens once, and if the condition isn't met, the thread waits. However, the potential problem is that if a spurious wakeup occurs (the thread wakes up even though the condition hasn't been met), the check will only happen once and may not properly handle the condition. Hence, we use while loop instead of if
        while(state.word != word) {
            state.cv.wait(lock);
        }
        cout << word <<endl;
        state.word = nextWord;
        state.cv.notify_all();
    }
};

int main() {
    State state("I");
    PrintWord i(state, "I", "am");
    PrintWord am(state, "am", "jigar");
    PrintWord jigar(state, "jigar", "Done");
    thread t2(am);
    thread t1(i);
    thread t3(jigar);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}
// Without threading
// #include<mutex>
// #include<condition_variable>
// #include<bits/stdc++.h>

// using namespace std;

// struct State {
//     string word;
//     State(const string& word): word(word) {}
// };

// class PrintWord {
//     State& state; // Make sure State& instead of State. Otherwise it will create new copy locally
//     string word;
//     string nextWord;
// public:
//     PrintWord(State& state, const string& word, const string& nextWord)
//     :state(state), word(word), nextWord(nextWord) {}
//     void operator()() {
//         while(state.word != word) {}
//         cout << word <<endl;
//         state.word = nextWord;
//     }
// };

// int main() {
//     State state("I");
//     PrintWord i(state, "I", "am");
//     PrintWord am(state, "am", "jigar");
//     PrintWord jigar(state, "jigar", "Done");
//     i();
//     am();
//     jigar();
//     return 0;
// }
