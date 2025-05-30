#pragma once

#include <optional>

template<typename Key, typename Value>
class Cache {
public:
    virtual ~Cache() = default;

    virtual bool set(const Key& key, const Value& value) = 0;
    virtual std::optional<Value> get(const Key& key) const = 0;
    virtual int size() const = 0;
    virtual bool isEmpty() const = 0;
    virtual void clear() = 0;
};
