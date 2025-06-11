#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <thread>
using namespace std;

// Forward declarations
class TokensBucket;

// Interface for refill rule
class IRefillRule {
public:
    virtual void refillBucket(TokensBucket& bucket, const string& entityId) = 0;
    virtual ~IRefillRule() = default;
};

// Token bucket class
class TokensBucket {
    int capacity;
    int tokens;

public:
    TokensBucket(int capacity_)
        : capacity(capacity_), tokens(capacity_) {}

    bool hasToken() const {
        return tokens > 0;
    }

    void consumeToken() {
        if (tokens > 0) --tokens;
    }

    void refill(int count) {
        tokens = min(capacity, tokens + count);
    }
};

// Constant rate refill rule
class ConstantRateRefillRule : public IRefillRule {
    int tokensToAdd;
    int windowMillis;
    unordered_map<string, long long> lastRefillMap;

    long long nowMillis() const {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now().time_since_epoch()).count();
    }

public:
    ConstantRateRefillRule(int tokensToAdd_, int windowMillis_)
        : tokensToAdd(tokensToAdd_), windowMillis(windowMillis_) {}

    void refillBucket(TokensBucket& bucket, const string& entityId) override {
        long long now = nowMillis();

        auto it = lastRefillMap.find(entityId);
        if (it == lastRefillMap.end()) {
            lastRefillMap[entityId] = now;
            return;
        }

        long long& lastRefill = it->second;
        if (now - lastRefill >= windowMillis) {
            bucket.refill(tokensToAdd);
            lastRefill = now;
        }
    }
};

// Interface for entities subject to rate limiting
class IRateLimitingEntity {
public:
    virtual string getId() const = 0;
    virtual shared_ptr<IRefillRule> getRefillRule() const = 0;
    virtual ~IRateLimitingEntity() = default;
};

// User entity
class User : public IRateLimitingEntity {
    string userId;
    shared_ptr<IRefillRule> refillRule;

public:
    User(const string& id)
        : userId(id),
          refillRule(make_shared<ConstantRateRefillRule>(5, 5000)) // 5 tokens per 5 seconds
    {}

    string getId() const override {
        return userId;
    }

    shared_ptr<IRefillRule> getRefillRule() const override {
        return refillRule;
    }
};

// API entity
class API : public IRateLimitingEntity {
    string apiName;
    shared_ptr<IRefillRule> refillRule;

public:
    API(const string& name)
        : apiName(name),
          refillRule(make_shared<ConstantRateRefillRule>(3, 3000)) // 3 tokens per 3 seconds
    {}

    string getId() const override {
        return apiName;
    }

    shared_ptr<IRefillRule> getRefillRule() const override {
        return refillRule;
    }
};

// Token bucket rate limiter
class TokenBucketRateLimiter {
    // Map entity ID to token bucket
    unordered_map<string, TokensBucket> buckets;

public:
    bool isRequestAllowed(const IRateLimitingEntity& entity) {
        string id = entity.getId();

        // If no bucket exists yet, create one with capacity = tokensToAdd (here 10 for demo)
        if (buckets.find(id) == buckets.end()) {
            buckets.emplace(id, TokensBucket(10)); // Capacity 10 tokens
        }

        TokensBucket& bucket = buckets.at(id);

        // Refill bucket based on entity's refill rule
        entity.getRefillRule()->refillBucket(bucket, id);

        if (!bucket.hasToken()) {
            return false;
        }

        bucket.consumeToken();
        return true;
    }
};

int main() {
    User user("user123");
    API api("getPosts");

    TokenBucketRateLimiter limiter;

    // Simulate rapid requests from user
    for (int i = 1; i <= 15; ++i) {
        bool allowed = limiter.isRequestAllowed(user);
        cout << "User request #" << i << " allowed: " << (allowed ? "Yes" : "No") << "\n";
    }

    // Simulate rapid API requests
    for (int i = 1; i <= 15; ++i) {
        bool allowed = limiter.isRequestAllowed(api);
        cout << "API request #" << i << " allowed: " << (allowed ? "Yes" : "No") << "\n";
    }

    // Wait 3 seconds to allow refill for user
    cout << "Waiting 3 seconds...\n";
    this_thread::sleep_for(chrono::seconds(3));

    for (int i = 1; i <= 7; ++i)
        cout << "User request after wait allowed: " << (limiter.isRequestAllowed(user) ? "Yes" : "No") << "\n";
    for (int i = 1; i <= 7; ++i)
        cout << "API request after wait allowed: " << (limiter.isRequestAllowed(api) ? "Yes" : "No") << "\n";

    // Wait 3 seconds to allow refill for user
    cout << "Waiting 3 seconds...\n";
    this_thread::sleep_for(chrono::seconds(3));

    for (int i = 1; i <= 7; ++i)
        cout << "User request after wait allowed: " << (limiter.isRequestAllowed(user) ? "Yes" : "No") << "\n";
    for (int i = 1; i <= 7; ++i)
        cout << "API request after wait allowed: " << (limiter.isRequestAllowed(api) ? "Yes" : "No") << "\n";

    return 0;
}
