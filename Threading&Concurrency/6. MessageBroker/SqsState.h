#pragma once
#include "CalculationMessage.h"
#include <vector>
using namespace std;

struct SqsState {
    vector<CalculationMessage> q;
    size_t offset = 0;
};
