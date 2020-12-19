#include "utils/threadpool.hpp"

#include <iostream>

namespace echidna::utils {
    ThreadPool::ThreadPool(size_t num_threads) : num_threads(num_threads), active(true) {
        this->threads = new std::thread[num_threads];

        for(size_t i = 0; i < num_threads; ++i) {
            this->threads[i] = std::thread(&ThreadPool::threadCallback, this);
        }
    }

    ThreadPool::~ThreadPool() {
        delete[] this->threads;
    }

    void ThreadPool::threadCallback() {
        while(true) {
            std::unique_lock lock(this->job_queue_mut);

            this->job_queue_var.wait(lock, [&] {return this->job_queue.size() > 0 || !this->active;});

            if(!this->active && this->job_queue.size() == 0)
                break;

            auto job = this->job_queue.front();
            this->job_queue.pop_front();

            lock.unlock();
            job();
        }
    }

    void ThreadPool::join() {
        for(size_t i = 0; i < this->num_threads; ++i) {
            this->threads[i].join();
        }
    }

    void ThreadPool::stop() {
        this->active = false;

        this->job_queue_var.notify_all();
    }

    void ThreadPool::drain() {
        std::unique_lock lock(this->job_queue_mut);

        this->job_queue_var.wait(lock, [&] {return this->job_queue.size() == 0;});
    }
}