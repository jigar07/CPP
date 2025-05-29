#pragma once
#include "CalculationMessage.h"

struct Tasks
{
    CalculationMessage message;
    long long millisEpoch;
    long long intervalMillis;

    // Comparison operator for priority_queue (min-heap behavior)
    bool operator<(const Tasks& other) const {
        return millisEpoch > other.millisEpoch;  // earlier times = higher priority
    }
};
