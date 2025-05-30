#pragma once

#include "Cache.h"
#include <unordered_map>
#include <list>
#include <optional>
#include <shared_mutex>

template<typename Key, typename Value>
class LRUCache : public Cache<Key, Value> {
public:
    explicit LRUCache(int maxSize) : maxSize_(maxSize) {}

    bool set(const Key& key, const Value& value) override {
        std::unique_lock lock(mutex_);
        auto it = nodes_.find(key);
        if (it != nodes_.end()) {
            // Update value and move to front
            it->second->value = value;
            itemsList_.splice(itemsList_.begin(), itemsList_, it->second);
            return true;
        }
        if (static_cast<int>(itemsList_.size()) >= maxSize_) {
            evictOneEntry();
        }
        itemsList_.emplace_front(key, value);
        nodes_[key] = itemsList_.begin();
        return true;
    }

    std::optional<Value> get(const Key& key) const override {
        std::shared_lock lock(mutex_);
        auto it = nodes_.find(key);
        if (it == nodes_.end()) {
            return std::nullopt;
        }
        // 需要升级为unique_lock以修改顺序
        lock.unlock();
        {
            std::unique_lock writeLock(mutex_);
            itemsList_.splice(itemsList_.begin(), itemsList_, it->second);
        }
        return it->second->value;
    }

    int size() const override {
        std::shared_lock lock(mutex_);
        return static_cast<int>(itemsList_.size());
    }

    bool isEmpty() const override {
        return size() == 0;
    }

    void clear() override {
        std::unique_lock lock(mutex_);
        nodes_.clear();
        itemsList_.clear();
    }

protected:
    // 不再加锁，由调用者负责加锁
    bool evictOneEntry() {
        if (itemsList_.empty()) {
            return false;
        }
        auto last = std::prev(itemsList_.end());
        nodes_.erase(last->key);
        itemsList_.erase(last);
        return true;
    }

private:
    struct CacheItem {
        Key key;
        Value value;
        CacheItem(const Key& k, const Value& v) : key(k), value(v) {}
    };

    int maxSize_;
    mutable std::shared_mutex mutex_;
    mutable std::list<CacheItem> itemsList_;
    std::unordered_map<Key, typename std::list<CacheItem>::iterator> nodes_;
};
