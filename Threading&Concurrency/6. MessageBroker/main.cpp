#include "MessageBroker.h"
#include "AdditionConsumer.h"
#include "MinusConsumer.h"
using namespace std;

int main() {
    MessageBroker broker;

    // Subscription 1: minus consumers
    broker.registerSubscription("minus-sub", {
        make_shared<MinusConsumer>(),
        make_shared<MinusConsumer>()
    });

    // Subscription 2: addition consumer
    broker.registerSubscription("add-sub", {
        make_shared<AdditionConsumer>()
    });

    broker.publish({10, 5});
    broker.publish({20, 7});
    broker.publish({8, 3});
    broker.publish({100, 20});

    // this_thread::sleep_for(chrono::seconds(20));
    cout<<"Finished!!!"<<endl;

    return 0;
}

// SQS Queue
// #include "SqsQueue.h"
// #include "AdditionConsumer.h"
// #include <memory>
// #include <chrono>
// #include <thread>
// using namespace std;

// int main() {
//     SqsQueue queue;

//     auto c1 = make_shared<AdditionConsumer>();
//     auto c2 = make_shared<AdditionConsumer>();

//     queue.registerConsumer(c1);
//     queue.registerConsumer(c2);

//     queue.publish({1, 2});
//     queue.publish({3, 4});
//     queue.publish({5, 6});
//     queue.publish({7, 8});
//     queue.publish({9, 10});

//     this_thread::sleep_for(chrono::seconds(20));

//     // Uncomment to test resetOffset
//     // queue.resetOffset(2);

//     return 0;
// }
