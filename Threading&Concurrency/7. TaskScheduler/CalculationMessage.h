#pragma once
#include <iostream>
using namespace std;

struct CalculationMessage {
    int a;
    int b;

    CalculationMessage(int a, int b) : a(a), b(b) {}

    friend ostream& operator<<(ostream& os, const CalculationMessage& msg) {
        os << "(" << msg.a << ", " << msg.b << ")";
        return os;
    }
};
