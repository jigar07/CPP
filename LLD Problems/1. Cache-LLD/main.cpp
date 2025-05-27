#include <iostream>
#include <map>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
using namespace std;

// EvictionPolicy Interface
template <typename K>
class EvictionPolicy {
public:
    virtual void markAccessed(const K& key) = 0;
    virtual void add(const K& key) = 0;
    virtual K evict() = 0;
    virtual ~EvictionPolicy() = default;
};

// https://leetcode.com/problems/lru-cache/description/
// LRU Eviction Policy
template <typename K>
class LRUEvictionPolicy : public EvictionPolicy<K> {
    list<K> accessOrder;
    unordered_map<K, typename list<K>::iterator> keyIteratorMap;

public:
    void markAccessed(const K& key) override {
        if (keyIteratorMap.find(key) != keyIteratorMap.end()) {
            accessOrder.erase(keyIteratorMap[key]);
        }
        add(key);
    }

    void add(const K& key) override {
        accessOrder.push_front(key);
        keyIteratorMap[key] = accessOrder.begin();
    }

    K evict() override {
        if (accessOrder.empty()) {
            throw runtime_error("Cache is empty");
        }
        K keyToEvict = accessOrder.back();
        accessOrder.pop_back();
        keyIteratorMap.erase(keyToEvict);
        return keyToEvict;
    }
};

// https://leetcode.com/problems/lfu-cache/description/)
template <typename K>
struct FrequencyNode {
    int freq;
    typename list<K>::iterator iter;

    // Default constructor required by unordered_map's operator[]
    FrequencyNode() : freq(0), iter() {}
    FrequencyNode(int f, typename list<K>::iterator it) : freq(f), iter(it) {}
};

// LFU Eviction Policy (simplified)
template <typename K>
class LFUEvictionPolicy : public EvictionPolicy<K> {
    unordered_map<K, FrequencyNode<K>> keyMeta;
    unordered_map<int, list<K>> freqListMap;
    int minFreq = 0;

public:
    void markAccessed(const K& key) override {
        int freq = keyMeta[key].freq;
        auto iter = keyMeta[key].iter;

        // Remove from current frequency list
        freqListMap[freq].erase(iter);

        // Increase frequency
        freq += 1;
        freqListMap[freq].push_front(key);
        keyMeta[key] = FrequencyNode<K>(freq, freqListMap[freq].begin());

        // Update minFreq if needed
        if (freqListMap[minFreq].empty()) {
            minFreq++;
        }
    }

    void add(const K& key) override {
        minFreq = 1;
        freqListMap[1].push_front(key);
        keyMeta[key] = FrequencyNode<K>(1, freqListMap[1].begin());
    }

    K evict() override {
        if (freqListMap[minFreq].empty()) {
            throw runtime_error("Cache is empty");
        }
        K keyToEvict = freqListMap[minFreq].back();
        freqListMap[minFreq].pop_back();
        keyMeta.erase(keyToEvict);
        return keyToEvict;
    }
};

// Storage Interface
template <typename K, typename V>
class Storage {
public:
    virtual void add(const K& key, const V& value) = 0;
    virtual V get(const K& key) = 0;
    virtual void remove(const K& key) = 0;
    virtual bool contains(const K& key) = 0;
    virtual size_t size() const = 0; // Added for size retrieval
    virtual ~Storage() = default;
};

// MapStorage Implementation
template <typename K, typename V>
class MapStorage : public Storage<K, V> {
    map<K, V> data;

public:
    void add(const K& key, const V& value) override {
        data[key] = value;
    }

    V get(const K& key) override {
        return data.at(key);
    }

    void remove(const K& key) override {
        data.erase(key);
    }

    bool contains(const K& key) override {
        return data.find(key) != data.end();
    }

    size_t size() const {
        return data.size();
    }
};

// Cache Implementation
template <typename K, typename V>
class Cache {
    unique_ptr<Storage<K, V>> storage;
    unique_ptr<EvictionPolicy<K>> policy;
    size_t maxSize;

public:
    Cache(Storage<K, V>* stor, EvictionPolicy<K>* pol, size_t size)
        : storage(stor), policy(pol), maxSize(size) {}

    void put(const K& key, const V& value) {
        if (storage->contains(key)) {
            policy->markAccessed(key);
        } else {
            if (maxSize > 0 && storage->size() >= maxSize) {
                K evictKey = policy->evict();
                storage->remove(evictKey);
            }
            policy->add(key);
        }
        storage->add(key, value);
    }

    V get(const K& key) {
        if (!storage->contains(key)) {
            throw runtime_error("Key not found");
        }
        policy->markAccessed(key);
        return storage->get(key);
    }
};


void testLRUCache() {
    auto mapStorage = make_unique<MapStorage<int, string>>();
    auto lruPolicy = make_unique<LRUEvictionPolicy<int>>();
    Cache<int, string> cache(mapStorage.release(), lruPolicy.release(), 3);

    cache.put(1, "One");
    cache.put(2, "Two");
    cache.put(3, "Three");

    cache.get(1); // Access key 1 to make it most recently used

    cache.put(4, "Four"); // This should evict the least recently used key (2)

    try {
        cout << "LRU Get 2: " << cache.get(2) << endl; // Should throw an error or undefined
    } catch (...) {
        cout << "LRU Key 2 was evicted!" << endl;
    }
}

void testLFUCache() {
    auto mapStorage = make_unique<MapStorage<int, string>>();
    auto lfuPolicy = make_unique<LFUEvictionPolicy<int>>();

    Cache<int, string> cache(mapStorage.release(), lfuPolicy.release(), 3);

    cache.put(1, "One");
    cache.put(2, "Two");
    cache.put(3, "Three");

    cache.get(1); // freq 2
    cache.get(1); // freq 3
    cache.get(2); // freq 2

    cache.put(4, "Four"); // Should evict key 3 (freq 1)

    try {
        cout << "LFU Get 3: " << cache.get(3) << endl;
    } catch (...) {
        cout << "LFU Key 3 was evicted!" << endl;
    }
}

int main() {
    testLRUCache();
    testLFUCache();

    return 0;
}
