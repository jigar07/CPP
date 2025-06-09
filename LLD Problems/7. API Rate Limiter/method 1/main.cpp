#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <thread>

// Forward declarations
class TokensBucket;

// Interface for refill rule
class IRefillRule {
public:
    virtual void refillBucket(TokensBucket& bucket, const std::string& entityId) = 0;
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
        tokens = std::min(capacity, tokens + count);
    }
};

// Constant rate refill rule
class ConstantRateRefillRule : public IRefillRule {
    int tokensToAdd;
    int windowMillis;
    std::unordered_map<std::string, long long> lastRefillMap;

    long long nowMillis() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

public:
    ConstantRateRefillRule(int tokensToAdd_, int windowMillis_)
        : tokensToAdd(tokensToAdd_), windowMillis(windowMillis_) {}

    void refillBucket(TokensBucket& bucket, const std::string& entityId) override {
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
    virtual std::string getId() const = 0;
    virtual std::shared_ptr<IRefillRule> getRefillRule() const = 0;
    virtual ~IRateLimitingEntity() = default;
};

// User entity
class User : public IRateLimitingEntity {
    std::string userId;
    std::shared_ptr<IRefillRule> refillRule;

public:
    User(const std::string& id)
        : userId(id),
          refillRule(std::make_shared<ConstantRateRefillRule>(5, 5000)) // 5 tokens per 5 seconds
    {}

    std::string getId() const override {
        return userId;
    }

    std::shared_ptr<IRefillRule> getRefillRule() const override {
        return refillRule;
    }
};

// API entity
class API : public IRateLimitingEntity {
    std::string apiName;
    std::shared_ptr<IRefillRule> refillRule;

public:
    API(const std::string& name)
        : apiName(name),
          refillRule(std::make_shared<ConstantRateRefillRule>(3, 3000)) // 3 tokens per 3 seconds
    {}

    std::string getId() const override {
        return apiName;
    }

    std::shared_ptr<IRefillRule> getRefillRule() const override {
        return refillRule;
    }
};

// Token bucket rate limiter
class TokenBucketRateLimiter {
    // Map entity ID to token bucket
    std::unordered_map<std::string, TokensBucket> buckets;

public:
    bool isRequestAllowed(const IRateLimitingEntity& entity) {
        std::string id = entity.getId();

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
        std::cout << "User request #" << i << " allowed: " << (allowed ? "Yes" : "No") << "\n";
    }

    // Simulate rapid API requests
    for (int i = 1; i <= 15; ++i) {
        bool allowed = limiter.isRequestAllowed(api);
        std::cout << "API request #" << i << " allowed: " << (allowed ? "Yes" : "No") << "\n";
    }

    // Wait 3 seconds to allow refill for user
    std::cout << "Waiting 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    for (int i = 1; i <= 7; ++i)
        std::cout << "User request after wait allowed: " << (limiter.isRequestAllowed(user) ? "Yes" : "No") << "\n";
    for (int i = 1; i <= 7; ++i)
        std::cout << "API request after wait allowed: " << (limiter.isRequestAllowed(api) ? "Yes" : "No") << "\n";

    // Wait 3 seconds to allow refill for user
    std::cout << "Waiting 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    for (int i = 1; i <= 7; ++i)
        std::cout << "User request after wait allowed: " << (limiter.isRequestAllowed(user) ? "Yes" : "No") << "\n";
    for (int i = 1; i <= 7; ++i)
        std::cout << "API request after wait allowed: " << (limiter.isRequestAllowed(api) ? "Yes" : "No") << "\n";

    return 0;
}
