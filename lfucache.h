#pragma once

#include "Cache.h"
#include <unordered_map>
#include <set>
#include <optional>
#include <atomic>
#include <shared_mutex>
#include <memory>

template<typename Key, typename Value>
class LFUCache : public Cache<Key, Value> {
public:
    explicit LFUCache(int maxSize) : maxSize_(maxSize) {}

    bool set(const Key& key, const Value& value) override {
        std::unique_lock lock(mutex_);
        auto it = items_.find(key);
        if (it != items_.end()) {
            auto item = it->second;
            keyPriorities_.erase(item);
            item->value = value;
            ++item->counter;
            keyPriorities_.insert(item);
            return true;
        } else {
            // 避免递归加锁，直接判断容器大小
            if (static_cast<int>(keyPriorities_.size()) >= maxSize_) {
                evictOneEntry();
            }
        }
        auto item = std::make_shared<CacheItem>(key, value);
        if (!keyPriorities_.insert(item).second) {
            return false;
        }
        items_[key] = item;
        return true;
    }

    std::optional<Value> get(const Key& key) const override {
        std::shared_lock lock(mutex_);
        auto it = items_.find(key);
        if (it == items_.end()) {
            return std::nullopt;
        }
        auto item = it->second;
        // 需要升级为unique_lock以修改优先级
        lock.unlock();
        {
            std::unique_lock writeLock(mutex_);
            keyPriorities_.erase(item);
            ++item->counter;
            keyPriorities_.insert(item);
        }
        return item->value;
    }

    int size() const override {
        // 避免递归加锁，直接返回容器大小
        std::shared_lock lock(mutex_);
        return static_cast<int>(keyPriorities_.size());
    }

    bool isEmpty() const override {
        return size() == 0;
    }

    void clear() override {
        std::unique_lock lock(mutex_);
        keyPriorities_.clear();
        items_.clear();
    }

protected:
    // 不再加锁，由调用者负责加锁
    bool evictOneEntry() {
        if (keyPriorities_.empty()) {
            return false;
        }
        auto it = keyPriorities_.begin();
        items_.erase((*it)->key);
        keyPriorities_.erase(it);
        return true;
    }

private:
    struct CacheItem {
        Key key;
        Value value;
        int counter;
        CacheItem(const Key& k, const Value& v)
            : key(k), value(v), counter(1) {}
        // 比较器：频率小的优先
        bool operator<(const std::shared_ptr<CacheItem>& other) const {
            if (counter != other->counter)
                return counter < other->counter;
            return key < other->key; // 保证唯一性
        }
    };

    int maxSize_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<Key, std::shared_ptr<CacheItem>> items_;
    struct CacheItemPtrLess {
        bool operator()(const std::shared_ptr<CacheItem>& a, const std::shared_ptr<CacheItem>& b) const {
            if (a->counter != b->counter)
                return a->counter < b->counter;
            return a->key < b->key;
        }
    };
    // 移除const修饰，允许成员被修改
    std::set<std::shared_ptr<CacheItem>, CacheItemPtrLess> keyPriorities_;
};
