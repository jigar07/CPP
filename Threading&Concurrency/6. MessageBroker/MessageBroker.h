#pragma once

#include "SqsQueue.h"
#include "IConsumer.h"
#include "CalculationMessage.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class MessageBroker {
    unordered_map<string, shared_ptr<SqsQueue>> queues;

public:
    // Register a subscription (queue) and its consumers
    void registerSubscription(const string& subscriptionName,
                              const vector<shared_ptr<IConsumer>>& consumers) {
        if (queues.find(subscriptionName) == queues.end()) {
            queues[subscriptionName] = make_shared<SqsQueue>();
        }

        auto& sqsQueue = queues[subscriptionName];
        for (const auto& consumer : consumers) {
            sqsQueue->registerConsumer(consumer);
        }
    }

    // Publish a message to all queues
    void publish(const CalculationMessage& msg) {
        for (const auto& [name, queue] : queues) {
            queue->publish(msg);
        }
    }

    // Reset offset for a specific subscription
    void resetOffset(const string& subscriptionName, size_t newOffset) {
        if (queues.find(subscriptionName) != queues.end()) {
            queues[subscriptionName]->resetOffset(newOffset);
        } else {
            cerr << "Subscription not found: " << subscriptionName << endl;
        }
    }
};
