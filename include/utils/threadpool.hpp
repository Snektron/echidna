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
    //A utility class defining a thread pool
    class ThreadPool {
        private:
            std::thread* threads;
            size_t num_threads;
            std::mutex job_queue_mut;
            std::condition_variable job_queue_var;
            std::deque<std::function<void()>> job_queue;

            std::atomic<bool> active;

            //Main routine run in threads
            void threadCallback();
        public:
            //Construct a thread pool with a given number of threads
            ThreadPool(size_t);
            ~ThreadPool();

            //Schedule a callback to run on the threadpool
            template <typename F>
            void schedule(F callback) {
                std::unique_lock lock(this->job_queue_mut);
                this->job_queue.push_back(callback);

                this->job_queue_var.notify_one();
            }

            //Drain the threadpool then stop all threads
            void stop();
            //Halt until all threads on the threadpool finish
            void join();
            //Complete all jobs in the current job queue
            void drain();
    };
}

#endif
