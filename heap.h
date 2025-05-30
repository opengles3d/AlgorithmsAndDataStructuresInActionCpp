#pragma once

#include <vector>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <algorithm>

template<typename T>
class DaryHeap {
public:
    explicit DaryHeap(int branchingFactor = MIN_BRANCHING_FACTOR)
        : branchingFactor_(branchingFactor) {
        if (branchingFactor_ < MIN_BRANCHING_FACTOR || branchingFactor_ > MAX_BRANCHING_FACTOR)
            throw std::invalid_argument("Branching factor must be between MIN_BRANCHING_FACTOR and MAX_BRANCHING_FACTOR");
    }

    DaryHeap(const std::vector<T>& elements, int branchingFactor = MIN_BRANCHING_FACTOR)
        : branchingFactor_(branchingFactor), elements_(elements) {
        if (branchingFactor_ < MIN_BRANCHING_FACTOR || branchingFactor_ > MAX_BRANCHING_FACTOR)
            throw std::invalid_argument("Branching factor must be between MIN_BRANCHING_FACTOR and MAX_BRANCHING_FACTOR");
        for (size_t i = 0; i < elements_.size(); ++i)
            positions_[elements_[i]] = static_cast<int>(i);
        heapify();
    }

    bool add(const T& element) {
        std::unique_lock lock(mutex_);
        if (positions_.count(element)) return false;
        elements_.push_back(element);
        positions_[element] = static_cast<int>(elements_.size()) - 1;
        bubbleUp(static_cast<int>(elements_.size()) - 1);
        return true;
    }

    std::optional<T> top() {
        std::unique_lock lock(mutex_);
        if (elements_.empty()) return std::nullopt;
        T topElem = elements_.front();
        removeAtIndexUnlocked(0); // 避免递归加锁
        return topElem;
    }

    std::optional<T> peek() const {
        std::shared_lock lock(mutex_);
        if (elements_.empty()) return std::nullopt;
        return elements_.front();
    }

    bool contains(const T& element) const {
        std::shared_lock lock(mutex_);
        return positions_.count(element) > 0;
    }

    bool remove(const T& element) {
        std::unique_lock lock(mutex_);
        auto it = positions_.find(element);
        if (it == positions_.end()) return false;
        int idx = it->second;
        removeAtIndexUnlocked(idx);
        return true;
    }

    bool updatePriority(const T& oldElement, const T& newElement) {
        std::unique_lock lock(mutex_);
        auto it = positions_.find(oldElement);
        if (it == positions_.end()) return false;
        int idx = it->second;
        elements_[idx] = newElement;
        positions_.erase(oldElement);
        positions_[newElement] = idx;
        if (!bubbleUp(idx))
            pushDown(idx);
        return true;
    }

    int size() const {
        std::shared_lock lock(mutex_);
        return static_cast<int>(elements_.size());
    }

    bool isEmpty() const {
        return size() == 0;
    }

    void clear() {
        std::unique_lock lock(mutex_);
        elements_.clear();
        positions_.clear();
    }

    // For testing: check heap invariants
    bool checkHeapInvariants() const {
        std::shared_lock lock(mutex_);
        for (int i = 0, n = static_cast<int>(elements_.size()); i < n; ++i) {
            int firstChild = getFirstChildIndex(i);
            int lastChild = std::min(getFirstChildIndex(i + 1), n);
            for (int j = firstChild; j < lastChild; ++j) {
                if (elements_[j] < elements_[i]) return false;
            }
        }
        return true;
    }

private:
    int getFirstChildIndex(int index) const {
        return branchingFactor_ * index + 1;
    }
    int getParentIndex(int index) const {
        return (index - 1) / branchingFactor_;
    }

    // Returns true if bubbled up, false if not
    bool bubbleUp(int idx) {
        bool moved = false;
        T elem = elements_[idx];
        while (idx > 0) {
            int parent = getParentIndex(idx);
            if (elements_[idx] < elements_[parent]) {
                std::swap(elements_[idx], elements_[parent]);
                positions_[elements_[idx]] = idx;
                positions_[elements_[parent]] = parent;
                idx = parent;
                moved = true;
            } else {
                break;
            }
        }
        return moved;
    }

    void pushDown(int idx) {
        int n = static_cast<int>(elements_.size());
        T elem = elements_[idx];
        while (true) {
            int firstChild = getFirstChildIndex(idx);
            if (firstChild >= n) break;
            int minChild = firstChild;
            int lastChild = std::min(firstChild + branchingFactor_, n);
            for (int j = firstChild + 1; j < lastChild; ++j) {
                if (elements_[j] < elements_[minChild])
                    minChild = j;
            }
            if (elements_[minChild] < elements_[idx]) {
                std::swap(elements_[idx], elements_[minChild]);
                positions_[elements_[idx]] = idx;
                positions_[elements_[minChild]] = minChild;
                idx = minChild;
            } else {
                break;
            }
        }
    }

    void heapify() {
        int n = static_cast<int>(elements_.size());
        for (int i = getParentIndex(n - 1); i >= 0; --i)
            pushDown(i);
    }

    void removeAtIndexUnlocked(int idx) {
        int lastIdx = static_cast<int>(elements_.size()) - 1;
        T removedElem = elements_[idx];
        if (idx != lastIdx) {
            std::swap(elements_[idx], elements_[lastIdx]);
            positions_[elements_[idx]] = idx;
        }
        positions_.erase(removedElem);
        elements_.pop_back();
        if (idx < static_cast<int>(elements_.size())) {
            if (!bubbleUp(idx))
                pushDown(idx);
        }
    }

    int branchingFactor_;
    std::vector<T> elements_;
    std::unordered_map<T, int> positions_;
    mutable std::shared_mutex mutex_;

    static inline const int MAX_BRANCHING_FACTOR = 10;
	static inline const int MIN_BRANCHING_FACTOR = 2;
};
