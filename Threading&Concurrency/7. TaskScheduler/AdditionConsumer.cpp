#include <bits/stdc++.h>
#include "AdditionConsumer.h"
using namespace std;

void AdditionConsumer::consume(const CalculationMessage& msg) {
    cout << this_thread::get_id() << ": Addition started: " << msg << endl;
    this_thread::sleep_for(chrono::seconds(5));
    cout << this_thread::get_id() << ": Addition finished: " << msg <<"==>" << msg.a + msg.b << endl;
}