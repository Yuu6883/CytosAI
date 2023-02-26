#include "pool.hpp"
#include "logger.hpp"

ThreadPool::ThreadPool(unsigned int n) : busy(0), processed(0), stop(0) {
    if (n <= 1) {
        sync_mode = true;
        logger::warn("Thread pool using sync mode\n");
    } else {
        sync_mode = false;
        for (unsigned int i = 0; i < n; ++i)
            workers.push_back(std::thread([&] { this->thread_proc(); }));
    }
}

ThreadPool::~ThreadPool() {
    if (sync_mode) return;
    
    // set stop-condition
    std::unique_lock<std::mutex> latch(queue_mutex);
    stop = true;
    cv_task.notify_all();
    latch.unlock();

    // all threads terminate, then we're done.
    for (auto& t : workers)
        t.join();
}

void ThreadPool::thread_proc() {
    while (true) {
        std::unique_lock<std::mutex> latch(queue_mutex);
        cv_task.wait(latch, [this]() { return stop || !tasks.empty(); });
        if (!tasks.empty()) {
            // got work. set busy.
            ++busy;

            // pull from queue
            auto fn = tasks.front();
            tasks.pop_front();

            // release lock. run async
            latch.unlock();

            // run function outside context
            fn();
            ++processed;

            latch.lock();
            --busy;
            cv_finished.notify_one();
        } else if (stop) break;
    }
}

void ThreadPool::enqueue(std::function<void(void)> f) {
    if (workers.size()) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace_back(std::forward<std::function<void(void)>>(f));
        cv_task.notify_one();
    } else f();
}

// waits until the queue is empty.
void ThreadPool::sync() {
    if (workers.size()) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv_finished.wait(lock, [this]() { return tasks.empty() && (busy == 0); });
    }
}