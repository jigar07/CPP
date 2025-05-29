#pragma once
# include "IConsumer.h"
class AdditionConsumer: public IConsumer {
    public:
    void consume(const CalculationMessage& msg);
};