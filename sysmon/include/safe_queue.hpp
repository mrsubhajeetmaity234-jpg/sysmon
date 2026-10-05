#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

namespace sysmon {

// Thread-safe producer/consumer queue (mutex + condition variable).
// pop() blocks until an item arrives; returns false once the queue is closed and drained.
template <typename T>
class SafeQueue {
public:
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(m_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [this] { return !q_.empty() || closed_; });
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(m_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool closed_ = false;
};

} // namespace sysmon
