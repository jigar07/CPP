#include "MyScheduler.h"
#include "AdditionConsumer.h"

using namespace std;

int main() {
    MyScheduler myTaskScheduler;
    AdditionConsumer a1, a2;
    myTaskScheduler.registerConsumer(a1);
    myTaskScheduler.registerConsumer(a2);
    while(true) {
        string input;
        getline(cin, input);

        istringstream stream(input);
        string operation;
        stream >> operation;

        if(operation == "exit") break;
        if(operation == "pub") {
            int a, b;
            long long startAfterInMillis, interValInMillis;
            stream >> a >> b >> startAfterInMillis >> interValInMillis;
            myTaskScheduler.scheduleAfter({a, b}, startAfterInMillis, interValInMillis);
        }
    }
    return 0;
}