#pragma once

#include <algorithm>
#include <iostream>
#include <deque>
#include <vector>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(unsigned int n);

    void enqueue(std::function<void(void)> f);
    void sync();
    inline unsigned int size() { return std::max(size_t(1), workers.size()); };
    ~ThreadPool();

    unsigned int getProcessed() const { return processed; }

private:
    std::vector<std::thread> workers;
    std::deque<std::function<void(void)>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv_task;
    std::condition_variable cv_finished;
    unsigned int busy;
    std::atomic_uint processed;
    bool sync_mode;
    bool stop;
    void thread_proc();
};