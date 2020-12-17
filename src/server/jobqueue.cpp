#include "server/jobqueue.hpp"

namespace echidna::server {
    JobQueue::JobQueue() : job_offset(0) {}

    JobQueue::~JobQueue() {}

    void JobQueue::addJob(const std::string& shader, uint32_t frames, uint32_t fps, uint32_t width, uint32_t height) {
        std::unique_lock(this->job_mutex);

        uint32_t job_id = ++this->job_offset;

        this->jobs[job_id] = new Job(shader, frames, fps, width, height);
        this->job_queue.push_back(job_id);

        this->job_wait.notify_all();
    }

    void JobQueue::addTasks(const std::vector<Task>& tasks) {
        std::unique_lock(this->job_mutex);
        for(const Task& t : tasks) {
            this->task_queue.push_back(t);
        }
    }

    std::vector<Task> JobQueue::getJobs(size_t max_jobs) {
        std::unique_lock lock(this->job_mutex);

        std::vector<Task> results;
        while(this->task_queue.size() > 0 && results.size() < max_jobs) {
            results.push_back(task_queue.front());
            task_queue.pop_front();
        }

        while(results.size() == 0) {
            this->job_wait.wait(lock, [&] {return this->job_queue.size() > 0;});

            while(this->job_queue.size() > 0 && results.size() < max_jobs) {
                Job& front_job = *jobs[job_queue.front()];

                while(front_job.hasTasks() && results.size() < max_jobs) {
                    results.push_back(front_job.getNextTask());
                }

                if(!front_job.hasTasks()) {
                    job_queue.pop_front();
                }
            }
        }

        return results;
    }
}