#pragma once
#include "CalculationMessage.h"

class IConsumer {
public:
    virtual void consume(const CalculationMessage& msg) = 0;
    virtual bool isFree() const = 0;
    virtual ~IConsumer() = default;
};
