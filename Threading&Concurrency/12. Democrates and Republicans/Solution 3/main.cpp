#include <iostream>
#include <queue>
#include <mutex>
#include <thread>

using namespace std;

struct Person {
    string name;
    int bathroomUseTime;
    Person(string name, int bathroomUseTime) : name(name), bathroomUseTime(bathroomUseTime) {}
};

struct Bathroom {
    int capacity;
    int currentDemocrates;
    int currentRepublicans;
};

struct State {
    Bathroom& bathroom;
    mutex mtx;
    condition_variable cv;
    queue<Person> democrates;
    queue<Person> republicans;
    State(Bathroom& bathroom) : bathroom(bathroom){};
};

void democrateWorker(State& state) {
    while(true) {
        Person person("", 0);
        {
            unique_lock<mutex> lock(state.mtx);
            state.cv.wait(lock, [&] () {
                return (state.democrates.size() > 0
                    && state.bathroom.currentRepublicans==0
                    && state.bathroom.currentDemocrates < state.bathroom.capacity);
            });
            person = state.democrates.front();
            state.democrates.pop();
            state.bathroom.currentDemocrates++;
            cout << person.name << " is using the bathroom" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(person.bathroomUseTime));
        {
            unique_lock<mutex> lock(state.mtx);
            state.bathroom.currentDemocrates--;
            state.cv.notify_all();
        }
    }
}

void republicanWorker(State& state) {
while(true) {
        Person person("", 0);
        {
            unique_lock<mutex> lock(state.mtx);
            state.cv.wait(lock, [&] () {
                return (state.republicans.size() > 0
                    && state.bathroom.currentDemocrates==0
                    && state.bathroom.currentRepublicans < state.bathroom.capacity);
            });
            person = state.republicans.front();
            state.republicans.pop();
            state.bathroom.currentRepublicans++;
            cout << person.name << " is using the bathroom" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(person.bathroomUseTime));
        {
            unique_lock<mutex> lock(state.mtx);
            state.bathroom.currentRepublicans--;
            state.cv.notify_all();
        }
    }
}

class BDR {
    State& state;
public:
    BDR(State& state) : state(state) {
    }

    void addDemocrate(Person person) {
        unique_lock<mutex> lock(state.mtx);
        state.democrates.push(person);
        state.cv.notify_one();
    }

    void addRepublican(Person person) {
        unique_lock<mutex> lock(state.mtx);
        state.republicans.push(person);
        state.cv.notify_one();
    }
};

int main() {
    Bathroom bathroom = { 3, 0, 0 };
    State state(bathroom);
    BDR bdr(state);

    // Worker threads
    for (int i = 0; i < 3; ++i) {
        // thread(democrateWorker, &state).detach();
        // thread(republicanWorker, &state).detach();
        // Above is giving error because You need to capture the function and arguments correctly, especially for non-copyable types like State&. Hence, used following.
        thread ([&]() {democrateWorker(state);}).detach();
        thread ([&]() {republicanWorker(state);}).detach();
    }

    this_thread::sleep_for(chrono::seconds(1));

    thread([&](){ bdr.addDemocrate({"D1", 3}); }).detach();
    thread([&](){ bdr.addDemocrate({"D2", 5}); }).detach();
    thread([&](){ bdr.addRepublican({"R1", 5}); }).detach();
    thread([&](){ bdr.addDemocrate({"D3", 5}); }).detach();
    thread([&](){ bdr.addDemocrate({"D4", 5}); }).detach();

    this_thread::sleep_for(chrono::seconds(30)); // Allow threads to finish
    {
        state.cv.notify_all(); // Wake up all threads
    }
    return 0;
}