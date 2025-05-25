#include <bits/stdc++.h>
#include <mutex>
#include <condition_variable>

using namespace std;
# define MAX_NUMBER 20

enum Turn {
    Odd,
    Even,
    Fib
};


struct State {
    Turn turn;
    mutex mtx;
    condition_variable cv;
    State(const Turn& turn): turn(turn) {}
};

class FibNumber {
    int currentNumber;
    int nextNumber;
    string seriesName;
    Turn turn;
    Turn nextTurn;
    State& state;
    int count=0;
public:
    FibNumber(State& state, const Turn& turn, const Turn& nextTurn, const string& seriesName) :
    state(state), turn(turn), nextTurn(nextTurn), seriesName(seriesName),  currentNumber(0), nextNumber(1) {
    }
    void run() {
        while(count < MAX_NUMBER) {
            unique_lock<mutex> lock(state.mtx);

            while(state.turn != turn) {
                state.cv.wait(lock);
            }
            cout<<seriesName<<": "<<currentNumber<<endl;
            int temp = nextNumber;
            nextNumber = currentNumber + nextNumber;
            currentNumber = temp;
            count++;
            state.turn = nextTurn;
            state.cv.notify_all();
        }
    }
};

class PrintNumber {
    int currentNumber;
    string seriesName;
    Turn turn;
    Turn nextTurn;
    State& state;
    int count=0;
public:
    PrintNumber(const int& startNumber, State& state, const Turn& turn, const Turn& nextTurn, const string& seriesName) :
    currentNumber(startNumber), state(state), turn(turn), nextTurn(nextTurn), seriesName(seriesName) {}
    void run() {
        while(count < MAX_NUMBER) {
            unique_lock<mutex> lock(state.mtx);

            while(state.turn != turn) {
                state.cv.wait(lock);
            }
            cout<<seriesName<<": "<<currentNumber<<endl;
            currentNumber += 2;
            count++;
            state.turn = nextTurn;
            state.cv.notify_all();
        }
    }
};

int main() {
    State state(Turn::Even);
    PrintNumber even(0, state, Turn::Even, Turn::Odd, "EvenSeries");
    PrintNumber odd(1, state,  Turn::Odd, Turn::Fib, "OddSeries");
    FibNumber fib(state, Turn::Fib, Turn::Even, "FibSeries");

    thread t1(&PrintNumber::run, &odd);
    thread t2(&PrintNumber::run, &even);
    thread t3(&FibNumber::run, &fib);

    t1.join();
    t2.join();
    t3.join();
    return 0;
}