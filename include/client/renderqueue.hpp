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
            RenderTaskInfo getTask();
    };
}

#endif
