#ifndef _ECHIDNA_CLIENT_RENDERQUEUE_HPP
#define _ECHIDNA_CLIENT_RENDERQUEUE_HPP

#include <deque>
#include <mutex>
#include <condition_variable>

#include "client/rendertask.hpp"

namespace echidna::client {
    //A basic queue containing job information for the client
    class RenderQueue {
        private:
            //The main queue
            std::deque<RenderTaskInfo> render_queue;

            //Synchronization
            std::mutex queue_mutex;
            std::condition_variable queue_var;
        public:
            RenderQueue();
            ~RenderQueue();

            //Add a new task to the queue
            void addTask(const RenderTaskInfo&);

            //Get a task from the queue, waiting if none are available
            template <typename F>
            RenderTaskInfo getTask(F job_request_callback) {
                //Wait until a task is available
                std::unique_lock lock(this->queue_mutex);
                this->queue_var.wait(lock, [&] {return this->render_queue.size() > 0;});

                //Get a task
                RenderTaskInfo result = this->render_queue.front();
                this->render_queue.pop_front();

                //Call the job-request callback if the queue is empty
                if(this->render_queue.size() == 0) {
                    lock.unlock();

                    job_request_callback();
                }
                return result;
            }
    };
}

#endif
