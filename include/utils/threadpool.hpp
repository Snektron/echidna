#ifndef _ECHIDNA_UTILS_THREADPOOL_HPP
#define _ECHIDNA_UTILS_THREADPOOL_HPP

#include <cstddef>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace echidna::utils {
    class ThreadPool {
        private:
            std::thread* threads;
            size_t num_threads;
            std::mutex job_queue_mut;
            std::condition_variable job_queue_var;
            std::deque<std::function<void()>> job_queue;

            std::atomic<bool> active;

            void threadCallback();
        public:
            ThreadPool(size_t);
            ~ThreadPool();

            template <typename F>
            void schedule(F callback) {
                std::unique_lock lock(this->job_queue_mut);
                this->job_queue.push_back(callback);

                this->job_queue_var.notify_one();
            }

            void stop();
            void join();
            void drain();
    };
}

#endif
