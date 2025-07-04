#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>
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

// Constant rate refill rule for token bucket
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

// Interface for rate limiter algorithms
class IRateLimiter {
public:
    virtual bool isRequestAllowed(const IRateLimitingEntity& entity) = 0;
    virtual ~IRateLimiter() = default;
};

// Token Bucket rate limiter implementation
class TokenBucketRateLimiter : public IRateLimiter {
    unordered_map<string, TokensBucket> buckets;

public:
    bool isRequestAllowed(const IRateLimitingEntity& entity) override {
        string id = entity.getId();

        if (buckets.find(id) == buckets.end()) {
            buckets.emplace(id, TokensBucket(10)); // capacity 10 tokens for demo
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

// Fixed Window rate limiter implementation
class FixedWindowRateLimiter : public IRateLimiter {
    struct Window {
        long long windowStart; // ms
        int count;
    };
    unordered_map<string, Window> windows;

    const int maxRequests = 10;
    const int windowSizeMillis = 5000;

    long long nowMillis() const {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now().time_since_epoch()).count();
    }

public:
    bool isRequestAllowed(const IRateLimitingEntity& entity) override {
        string id = entity.getId();
        long long now = nowMillis();

        auto it = windows.find(id);
        if (it == windows.end() || now - it->second.windowStart >= windowSizeMillis) {
            // Start a new window
            windows[id] = { now, 1 };
            return true;
        }

        Window& window = windows[id];
        if (window.count < maxRequests) {
            window.count++;
            return true;
        }

        return false;
    }
};

int main() {
    User user("user123");
    API api("getPosts");

    unique_ptr<IRateLimiter> tokenBucketLimiter = make_unique<TokenBucketRateLimiter>();
    unique_ptr<IRateLimiter> fixedWindowLimiter = make_unique<FixedWindowRateLimiter>();

    cout << "Using Token Bucket Rate Limiter\n";
    for (int i = 1; i <= 15; ++i) {
        bool allowedUser = tokenBucketLimiter->isRequestAllowed(user);
        bool allowedApi = tokenBucketLimiter->isRequestAllowed(api);
        cout << "User request #" << i << " allowed: " << (allowedUser ? "Yes" : "No") << ", "
                  << "API request #" << i << " allowed: " << (allowedApi ? "Yes" : "No") << "\n";
    }

    cout << "\nWaiting 6 seconds to allow refill...\n\n";
    this_thread::sleep_for(chrono::seconds(6));

    for (int i = 1; i <= 7; ++i) {
        bool allowedUser = tokenBucketLimiter->isRequestAllowed(user);
        bool allowedApi = tokenBucketLimiter->isRequestAllowed(api);
        cout << "User request after wait #" << i << " allowed: " << (allowedUser ? "Yes" : "No") << ", "
                  << "API request after wait #" << i << " allowed: " << (allowedApi ? "Yes" : "No") << "\n";
    }

    cout << "\nUsing Fixed Window Rate Limiter\n";
    for (int i = 1; i <= 15; ++i) {
        bool allowedUser = fixedWindowLimiter->isRequestAllowed(user);
        bool allowedApi = fixedWindowLimiter->isRequestAllowed(api);
        cout << "User request #" << i << " allowed: " << (allowedUser ? "Yes" : "No") << ", "
                  << "API request #" << i << " allowed: " << (allowedApi ? "Yes" : "No") << "\n";
    }

    return 0;
}
