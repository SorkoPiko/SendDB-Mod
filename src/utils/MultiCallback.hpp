#ifndef SENDDB_MULTICALLBACK_HPP
#define SENDDB_MULTICALLBACK_HPP

#include <functional>
#include <set>

class MultiCallback {
    std::set<int> calledIndices;
    size_t totalCalls;
    std::function<void()> finalCallback;
    std::mutex mtx;

public:
    template <typename Func>
    MultiCallback(const Func&& callback, const size_t totalCalls) : totalCalls(totalCalls), finalCallback(callback) {
        if (totalCalls == 0) finalCallback();
    }

    void operator()(const int index) {
        std::lock_guard lock(mtx);
        if (calledIndices.contains(index)) return;

        calledIndices.insert(index);
        if (calledIndices.size() == totalCalls) finalCallback();
    }
};

#endif