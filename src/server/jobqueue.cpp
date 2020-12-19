#include "server/jobqueue.hpp"

namespace echidna::server {
    JobQueue::JobQueue() : job_offset(0) {}

    JobQueue::~JobQueue() {}

    void JobQueue::addJob(const std::string& shader, uint32_t frames, uint32_t fps, uint32_t width, uint32_t height) {
        std::unique_lock lock(this->job_mutex);
        std::unique_lock lock2(this->job_map_mutex);

        uint32_t job_id = ++this->job_offset;

        this->jobs[job_id] = new Job(shader, frames, fps, width, height);
        this->job_queue.push_back(job_id);

        this->job_wait.notify_all();
    }

    void JobQueue::addTasks(const std::vector<Task>& tasks) {
        std::unique_lock lock(this->job_mutex);
        for(const Task& t : tasks) {
            this->task_queue.push_back(t);
        }

        this->job_wait.notify_all();
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
                std::shared_lock lock2(this->job_map_mutex);
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


    Job* JobQueue::findJob(uint32_t job_id) {
        std::shared_lock lock(this->job_map_mutex);
        if(this->jobs.count(job_id) == 0)
            return nullptr;
        return this->jobs[job_id];
    }

    void JobQueue::finishJob(uint32_t job_id) {
        {
            std::unique_lock lock(this->job_map_mutex);
            if(this->jobs.count(job_id) == 0)
                return;

            Job* job = this->jobs[job_id];
            delete job;

            this->jobs.erase(job_id);
        }


        std::unique_lock lock2(this->finished_mutex);
        this->finished_jobs.push_back(job_id);
    }
}