# Problem: API Rate Limiter
- Note: This problem is difficult. Implement it again

https://enginebogie.com/public/question/design-api-rate-limiter/128

Imagine we are building an application that is used by many different customers. We want to avoid one customer being able to overload the system by sending too many requests, so we enforce a per-customer rate limit. The rate limit is defined as:
"Each customer can make X requests per Y seconds"

Perform rate limiting logic for provided customer ID. Return true if the request is allowed, and false if it is not.
------
## Thought process
- isRequestAllowed, refill are the core functionality for the class.
- **IRateLimitingEntity**: entity based on which rate limiting should be apply
- **IRateLimiter**: Rate limiter algo such as tocket bucket, sliding window, fixed window
  - RateLimiter stores limiter in map based on id.
  - For ex:
    - `TokenBucketRateLimiter` has `unordered_map<string, TokensBucket> buckets;`. So, for each id token bucket is stored. And, this token bucket is checked to find out if it is token or not
    - `FixedWindowRateLimiter` has `unordered_map<string, Window> windows;`
- **IRefillRule**: Rule for refill
- Interfaces: IRateLimitingEntity, IRefillRule, IRateLimiter
- Classes: User, API, TokenBucket, TokenBucketRateLimiter, ConstantRateRefillRule


## Algorithms
Finalize the rate limiting algorithm.
	- Fixed Window - Fix time
	- Sliding Window - Last Y seconds or minutes
	- Token Bucket - every `time window`, you refill the bucket with that many tokens.

- Fixed Window
0
1-Y: X
Y+1 - 2Y: X

- Token Bucket
bucket: tokens
every `time window`, you refill the bucket with that many tokens.


## 3 steps:
- core model classes: bottom - start with the core of the solution
- properties: bottom - start with the core of the solution
- methods - top down

### Design
- **IRateLimiter** interface
- TokenBucket
  - TokenBucketRateLimiter
    - IsRequestAllowed
      - **IRateLimitingEntity** - gives the ID(UserId, API, IP address etc) based on which rate limiter would be applied
      - IsRequestAllowed takes IRateLimitingEntity as parameter
- Token - Not used anywhere, we just created it
- **TokensBucket** class
- **IRefillRule** created
  - Each entity will give rule for refill
- In this begining start with simple methods. Then based on requirement make OCP complient
- If primary allows then use it else use fallback (Fallback is like to get burst traffic functionality)
  - FallbackRateLimiter class

## Open items:
* Different entity than customer.
  * Rate limit per API.
  * Rate limit per user per API
  * Rate limit globally.
* Different refill rate for different customers/entities.

```java

interface IRateLimitingEntity {
	String getRateLimitingId();
	IRefillRule getRefillRule();
}

class User implements IRateLimitingEntity {
	String userId;

	String getRateLimitingId() {
		return userId;
	}

	IRefillRule getRefillRule() {
		return new ConstantRateRefillRule(10, 20);
	}
}

class API implements IRateLimitingEntity {
	String apiName;

	String getRateLimitingId() {
		return apiName;
	}
}

class UserPerAPI implements IRateLimitingEntity {
	User user;
	API api;

	String getRateLimitingId() {
		return user.getRateLimitingId() + "-" + api.getRateLimitingId();
	}
}

class Global implements IRateLimitingEntity {

	String getRateLimitingId() {
		return "global";
	}
}

class MultiRuleEntityUserPerAPI implements IRateLimitingEntity {
	User user;
	API api;

	String getRateLimitingId() {
		return user.getRateLimitingId() + "-" + api.getRateLimitingId();
	}

	IRefillRule getRefillRule() {
		return user.getRateLimitingId() + "-" + api.getRateLimitingId();
	}
}

interface IRefillRule {
	void refillBucket(TokensBucket bucket);
}

class ConstantRateRefillRule implements IRefillRule {

	private void Integer tokensToRefill;
	private void Integer tokensRefillWindowInMillis;

	void refillBucket(TokensBucket bucket) {
		Integer lastRefillTimestampInMillis = bucketLastRefillTimestampInMillis.get(bucket);
		while(lastRefillTimestampInMillis + tokensRefillWindowInMillis < System.currentMillis()) {
			bucket.refill((tokensToRefill));
			lastRefillTimestampInMillis += tokensRefillWindowInMillis;
		}
	}
}

class MorningEveningRefillRule implements IRefillRule {

	IRefillRule morningRule;
	IRefillRule eveningRule;

	void refillBucket(TokensBucket bucket) {
		if (isMorningRightNow()) {
			morningRule.refillBucket(bucket);
		} else {
			eveningRule.refillBucket(bucket);
		}
	}

	isMorningRightNow() {

	}
}

class DatTimebasedRefillRule implements IRefillRule {

	Map<Time, IRefillRule> rules;

	
	void refillBucket(TokensBucket bucket) {
	}
}

class FetchTopKAPI {

	IRateLimiter rl = new FallbackRateLimiter(new TokenBucketRateLimiter(), new SlidingWindowRateLimiter());

	public void fetchTopK() {

	}
}

class PremiumUser {


	getRefillRule() {
		return new ConstantRateRefillRule(10, 5);
	}
}

class PostAPI {


	getRefillRule() {
		return new ConstantRateRefillRule(1, 2);
	}
}

interface IRateLimiter {
	boolean isRequestAllowed(String entityId);
}

class FallbackRateLimiter implements IRateLimiter {
	private IRateLimiter primary;
	private IRateLimiter fallback;

	boolean isRequestAllowed(IRateLimitingEntity entity) {
		boolean primaryResp = primary.isRequestAllowed(entity);
		if (primaryResp) {
			return primaryResp;
		}
		return fallback.isRequestAllowed(entity)
	}

}

/*
User: 10 per 20 seconds
per client: 5 per 20 seconds

API: 10 per 20 seconds in the morning & 30 per 5 seconds in the evening;



token bucket
sliding
*/

class TokenBucketRateLimiter implements IRateLimiter {
	// With following two properties our code is not extensible on refill rate and refill at different time if any user want to configure it.
	// private void Integer tokensToRefill;
	// private void Integer tokensRefillWindowInMillis;

	private void Map<TokensBucket, Integer> bucketLastRefillTimestampInMillis;
	private Map<String, TokensBucket> buckets;

	boolean isRequestAllowed(IRateLimitingEntity entity) {
		final String entityId = entity.getRateLimitingId();
		final TokensBucket bucket = buckets.get(entityId);
		// refillBucket(bucket);
		entity.getRefillRule().refillBucket(bucket);
		// Optional<Token> token = bucket.giveToken();
		// if (token.isEmpty()) {
		// 	return false;
		// }

		boolean result = bucket.hasToken();
		if (result) {
			bucket.reduceToken()
		}
		return result;
	}


	// void refillBucket(TokensBucket bucket) {
	// 	Integer lastRefillTimestampInMillis = bucketLastRefillTimestampInMillis.get(bucket);
	// 	while(lastRefillTimestampInMillis + tokensRefillWindowInMillis < System.currentMillis()) {
	// 		bucket.refill((tokensToRefill));
	// 		lastRefillTimestampInMillis += tokensRefillWindowInMillis;
	// 	}
	// }

}



class TokensBucket {
	private Integer maxSize;
	private Integer currentSize;
	// private Integer List<Token> tokens;

	// Optional<Token> giveToken() {
	// 	currentSize--;
	// 	return new Token();
	// }

	void refill(Integer refillCount) {
		currentSize = Math.min(maxSize, currentSize + refillCount);
	}

	boolean hasToken() {
		return currentSize > 0;
	}

	void reduceToken() {
		currentSize--;
	}

}

class Token {

}
```
