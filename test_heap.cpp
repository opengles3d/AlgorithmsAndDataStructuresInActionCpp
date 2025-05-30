#include "heap.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <string>

void test_basic_add_peek_top() {
    DaryHeap<int> heap(3);
    assert(heap.isEmpty());
    assert(heap.size() == 0);
    assert(!heap.peek().has_value());
    assert(!heap.top().has_value());

    assert(heap.add(5));
    assert(heap.add(3));
    assert(heap.add(7));
    assert(heap.add(1));
    assert(!heap.add(3)); // duplicate

    assert(heap.size() == 4);
    assert(heap.peek().value() == 1);

    auto t = heap.top();
    assert(t.has_value() && t.value() == 1);
    assert(heap.size() == 3);
    assert(heap.peek().value() == 3);
}

void test_remove() {
    DaryHeap<int> heap(4);
    heap.add(10);
    heap.add(20);
    heap.add(5);
    heap.add(15);

    assert(heap.remove(20));
    assert(!heap.remove(100)); // not present
    assert(heap.size() == 3);
    assert(heap.contains(10));
    assert(!heap.contains(20));
}

void test_updatePriority() {
    DaryHeap<int> heap(2);
    heap.add(10);
    heap.add(20);
    heap.add(30);

    // 10 is root, update 30 to 5, should bubble up
    assert(heap.updatePriority(30, 5));
    assert(heap.peek().value() == 5);

    // update 5 to 50, should push down
    assert(heap.updatePriority(5, 50));
    assert(heap.peek().value() == 10);

    // update non-existent
    assert(!heap.updatePriority(100, 1));
}

void test_clear_and_invariants() {
    DaryHeap<int> heap(3);
    for (int i = 0; i < 100; ++i) heap.add(i);
    assert(heap.size() == 100);
    assert(heap.checkHeapInvariants());
    heap.clear();
    assert(heap.isEmpty());
    assert(heap.size() == 0);
    assert(!heap.peek().has_value());
    assert(!heap.top().has_value());
}

void test_heapify_constructor() {
    std::vector<int> v = {7, 3, 9, 1, 5};
    DaryHeap<int> heap(v, 3);
    assert(heap.size() == 5);
    assert(heap.checkHeapInvariants());
    assert(heap.peek().value() == 1);
}

void test_string_heap() {
    DaryHeap<std::string> heap(2);
    heap.add("pear");
    heap.add("apple");
    heap.add("banana");
    assert(heap.peek().value() == "apple");
    heap.remove("apple");
    assert(heap.peek().value() == "banana");
}

void test_edge_cases() {
    // branching factor out of range
    try {
        DaryHeap<int> heap(1);
        assert(false);
    } catch (const std::invalid_argument&) {}

    try {
        DaryHeap<int> heap(11);
        assert(false);
    } catch (const std::invalid_argument&) {}

    // remove from empty
    DaryHeap<int> heap2(2);
    assert(!heap2.remove(1));
    assert(!heap2.updatePriority(1, 2));
}

int main() {
    test_basic_add_peek_top();
    test_remove();
    test_updatePriority();
    test_clear_and_invariants();
    test_heapify_constructor();
    test_string_heap();
    test_edge_cases();
    std::cout << "All DaryHeap tests passed!" << std::endl;
    return 0;
}
