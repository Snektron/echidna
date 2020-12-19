#include "client/renderqueue.hpp"

namespace echidna::client {
    RenderQueue::RenderQueue() {}
    RenderQueue::~RenderQueue() {}

    void RenderQueue::addTask(const RenderTaskInfo& task) {
        std::unique_lock lock(this->queue_mutex);
        this->render_queue.push_back(task);

        this->queue_var.notify_one();
    }
}