#ifndef _ECHIDNA_CLIENT_RENDERQUEUE_HPP
#define _ECHIDNA_CLIENT_RENDERQUEUE_HPP

#include <deque>
#include <mutex>
#include <condition_variable>

#include "client/rendertask.hpp"

namespace echidna::client {
    class RenderQueue {
        private:
            std::deque<RenderTaskInfo> render_queue;
            std::mutex queue_mutex;
            std::condition_variable queue_var;
        public:
            RenderQueue();
            ~RenderQueue();

            void addTask(const RenderTaskInfo&);

            template <typename F>
            RenderTaskInfo getTask(F job_request_callback) {
                std::unique_lock lock(this->queue_mutex);
                this->queue_var.wait(lock, [&] {return this->render_queue.size() > 0;});

                RenderTaskInfo result = this->render_queue.front();
                this->render_queue.pop_front();

                if(this->render_queue.size() == 0) {
                    lock.unlock();

                    job_request_callback();
                }
                return result;
            }
    };
}

#endif
