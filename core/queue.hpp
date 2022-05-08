#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
namespace core {
template <typename T>
struct ThreadSafeQueue
{
    ThreadSafeQueue() {}

    void push(T e)
    {
        std::unique_lock<std::mutex> uniq_lock(mutex_);
        real_queue_.emplace(e);
        cond_.notify_one();
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> uniq_lock(mutex_);
        cond_.wait(uniq_lock, [q = this] { return q->m_must_return_nullptr || !q->real_queue_.empty(); });

        if (m_must_return_nullptr)
            return {};

        T ret = real_queue_.front();
        real_queue_.pop();

        return ret;
    }

    std::queue<T>& destroy()
    {
        // Even flag should under lock
        // https://stackoverflow.com/a/38148447/4144109
        std::unique_lock<std::mutex> _(mutex_);
        m_must_return_nullptr.exchange(true);
        cond_.notify_all();

        // No element will be taken from the queue
        // So we return the queue in case the caller need them
        // WARNING: put may happen after this operation, caller should think about this
        return real_queue_;
    }

private:
    std::queue<T> real_queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

    std::atomic<bool> m_must_return_nullptr;
};
}