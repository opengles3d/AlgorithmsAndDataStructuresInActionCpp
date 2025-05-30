#include "lrucache.h"
#include <cassert>
#include <iostream>
#include <string>

void test_basic_set_get() {
    LRUCache<int, std::string> cache(3);
    assert(cache.set(1, "one"));
    assert(cache.set(2, "two"));
    assert(cache.set(3, "three"));
    assert(cache.size() == 3);

    auto v1 = cache.get(1);
    assert(v1.has_value() && v1.value() == "one");
    auto v2 = cache.get(2);
    assert(v2.has_value() && v2.value() == "two");
    auto v3 = cache.get(3);
    assert(v3.has_value() && v3.value() == "three");
}

void test_update_value() {
    LRUCache<int, std::string> cache(2);
    cache.set(1, "one");
    cache.set(2, "two");
    assert(cache.get(1).value() == "one");
    assert(cache.set(1, "uno"));
    assert(cache.get(1).value() == "uno");
}

void test_eviction() {
    LRUCache<int, std::string> cache(2);
    cache.set(1, "one");
    cache.set(2, "two");
    // Access 1 to make it most recently used
    cache.get(1);
    // Insert 3, should evict 2 (least recently used)
    cache.set(3, "three");
    assert(!cache.get(2).has_value());
    assert(cache.get(1).has_value());
    assert(cache.get(3).has_value());
}

void test_clear_and_empty() {
    LRUCache<int, std::string> cache(2);
    cache.set(1, "one");
    cache.set(2, "two");
    assert(!cache.isEmpty());
    cache.clear();
    assert(cache.isEmpty());
    assert(cache.size() == 0);
    assert(!cache.get(1).has_value());
}

void test_eviction_order() {
    LRUCache<int, std::string> cache(2);
    cache.set(1, "one");
    cache.set(2, "two");
    // Access 1, so 2 is LRU
    cache.get(1);
    cache.set(3, "three");
    assert(!cache.get(2).has_value());
    assert(cache.get(1).has_value());
    assert(cache.get(3).has_value());
    // Now 1 is MRU, 3 is LRU
    cache.set(4, "four");
    assert(!cache.get(3).has_value());
    assert(cache.get(1).has_value());
    assert(cache.get(4).has_value());
}

int main() {
    test_basic_set_get();
    test_update_value();
    test_eviction();
    test_clear_and_empty();
    test_eviction_order();
    std::cout << "All LRUCache tests passed!" << std::endl;
    return 0;
}
