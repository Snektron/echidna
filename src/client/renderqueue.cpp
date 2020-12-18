#include "client/renderqueue.hpp"

namespace echidna::client {
    RenderQueue::RenderQueue() {}
    RenderQueue::~RenderQueue() {}

    void RenderQueue::addTask(const RenderTaskInfo& task) {
        std::unique_lock lock(this->queue_mutex);
        this->render_queue.push_back(task);

        this->queue_var.notify_one();
    }

    RenderTaskInfo RenderQueue::getTask() {
        std::unique_lock lock(this->queue_mutex);
        this->queue_var.wait(lock, [&] {return this->render_queue.size() > 0;});

        RenderTaskInfo result = this->render_queue.front();
        this->render_queue.pop_front();
        return result;
    }
}