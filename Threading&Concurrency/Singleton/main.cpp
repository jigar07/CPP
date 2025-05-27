#include <bits/stdc++.h>
#include <mutex>
using namespace std;


// 1. pointer to constant integer
// const int* 
// int const*
// int const* ptr = nullptr;
// int a = 5;
// ptr = &a;    // ✅ You can change the pointer to point to different addresses
// *ptr = 10;   // ❌ Error: Cannot modify the value of a through ptr

// 2. constant pointer to integer
// int* const
// int a = 5;
// int b = 10;
// int* const ptr = &a;  // Pointer is constant, must be initialized

// ptr = &b;   // ❌ Error: Cannot change the address the pointer is pointing to
// *ptr = 20;  // ✅ You can change the value at the pointed address (a becomes 20)


// 3. constant pointer to constant integer
// const int* const
// int const* const
// int a = 5;
// const int* const ptr = &a;  // Both pointer and the value are constant

// ptr = &a;    // ❌ Error: Cannot change the pointer
// *ptr = 10;   // ❌ Error: Cannot change the value at ptr


class Singleton {
    Singleton() {
        cout<< "Singleton created" <<endl;
    }

    // prevent copy
    Singleton(const Singleton&) = delete;

    // prevent assignment
    Singleton& operator=(const Singleton&) = delete;

    // For eager initialization `Static Singleton* singleton = new singleton()`; For eager initialization thready safety not required
    // In C++ prior to C++17, non-const static data members cannot have in-class initializers unless they are:
        // const and
        // of integral or enum type or int, char, bool etc
        // So `static Singleton* singleton = nullptr;` line violates this rule.//
        // `inline static Singleton* singleton = nullptr; // ✅ C++17 and later` can be done
        // Valid examples
            // class MyClass {
            //     static const int MAX = 100;       // ✅ const + integral
            //     static constexpr double PI = 3.14; // ✅ with constexpr (C++11+)
            // };

    // lazy initialization.
    // static std::unique_ptr<Singleton> singletonInstanceUsingunique_ptr;
    static Singleton* singleton;
    static mutex mtx;
public:
    static Singleton& getInstanceNotThreadSafe() {
        if(singleton == NULL) {
            singleton = new Singleton();
        }
        return *singleton;
    }
    static Singleton& getInstance() {
        lock_guard<mutex> lock(mtx);
        if(singleton == NULL) {
            singleton = new Singleton();
        }
        return *singleton;
    }

    // following is best
    static Singleton& getInstanceThreadSafeWithDoubleCheckedLocking() {
        if (singleton == nullptr) {               // 1st check (without lock)
            std::lock_guard<std::mutex> lock(mtx); 
            if (singleton == nullptr) {           // 2nd check (with lock)
                singleton = new Singleton();
            }
        }
        return *singleton;
    }

    // Use C++11’s local static variable for better thread-safe lazy initialization:
    // C++11 guarantees that function-local static variables are initialized exactly once, even when multiple threads call getInstance() concurrently.
    static Singleton& getInstance2() {
        static Singleton instance;  // Initialized once, thread-safe in C++11+
        return instance;
    }

    // static Singleton& getInstance3() {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     if (singletonInstanceUsingunique_ptr == NULL) {
    //         singletonInstanceUsingunique_ptr.reset(new Singleton());
    //     }
    //     return *singletonInstanceUsingunique_ptr;
    // }

};
// Define and initialize all static member if not initialized in class. If we dont do this then will get `undefined reference to `Singleton::singleton'` error
Singleton* Singleton::singleton = nullptr;
mutex Singleton::mtx;
// std::unique_ptr<Singleton> Singleton::singletonInstanceUsingunique_ptr = nullptr;
// Thread function to get singleton instance pointer
void threadFunc(set<Singleton*>& instances, mutex& outputMtx) {
    Singleton& instance = Singleton::getInstanceThreadSafeWithDoubleCheckedLocking();
    {
        lock_guard<mutex> lock(outputMtx);
        instances.insert(&instance);
    }
}

void accessSingleton() {
    Singleton& instance = Singleton::getInstanceThreadSafeWithDoubleCheckedLocking();
    cout << "Instance address: " << &instance << endl;
}

int main() {
    // Singleton& s1 = Singleton::getInstanceThreadSafeWithDoubleCheckedLocking();
    // Singleton& s2 = Singleton::getInstanceThreadSafeWithDoubleCheckedLocking();
    // if(&s1 == &s2)
    //     cout<<"both instances are same"<<endl;
    // else
    //     cout<<"both instances are not same"<<endl;
    thread t1(accessSingleton);
    thread t2(accessSingleton);
    thread t3(accessSingleton);
    thread t4(accessSingleton);
    t1.join(), t2.join(), t3.join(), t4.join();

    const int numThreads = 10;
    vector<thread> threads;
    set<Singleton*> instances;
    mutex outputMtx;

    for (int i = 0; i < numThreads; ++i) {
        // emplace_back more efficient, because it constructs the object directly inside the container, avoiding unnecessary copies or moves.
        threads.emplace_back(threadFunc, ref(instances), ref(outputMtx));
    }

    for (auto& t : threads) {
        t.join();
    }

    if (instances.size() == 1) {
        cout << "All threads got the same instance." << endl;
    } else {
        cout << "Different instances detected!" << endl;
    }

    return 0;
}
