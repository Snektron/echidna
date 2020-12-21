#include "server/jobqueue.hpp"
#include "utils/log.hpp"

#include <chrono>

namespace echidna::server {
    JobQueue::JobQueue() : job_offset(0) {}

    JobQueue::~JobQueue() {}

    uint32_t JobQueue::addJob(const std::string& shader, uint32_t frames, uint32_t fps, uint32_t width, uint32_t height) {
        std::unique_lock lock(this->job_mutex);
        std::unique_lock lock2(this->job_map_mutex);

        uint32_t job_id = this->job_offset++;

        this->jobs[job_id] = new Job(shader, frames, fps, width, height);
        this->job_queue.push_back(job_id);

        auto start = std::chrono::system_clock::now();
        log::write("Created job ", job_id, " at ", std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count(), " ms");

        this->job_wait.notify_all();
        return job_id;
    }

    void JobQueue::addTasks(const std::vector<Task>& tasks) {
        std::unique_lock lock(this->job_mutex);
        for(const Task& t : tasks) {
            this->task_queue.push_back(t);
        }

        this->job_wait.notify_all();
    }

    std::vector<Task> JobQueue::getJobs(size_t max_jobs) {
        std::vector<Task> results;
        std::unique_lock lock(this->job_mutex);

        //Wait until a job is available
        this->job_wait.wait(lock, [&] {return this->job_queue.size() > 0 || this->task_queue.size() > 0;});

        //Check the priority queue first
        while(this->task_queue.size() > 0 && results.size() < max_jobs) {
            results.push_back(task_queue.front());
            task_queue.pop_front();
        }
        //Check for regular jobs
        while(this->job_queue.size() > 0 && results.size() < max_jobs) {
            std::shared_lock lock2(this->job_map_mutex);
            uint32_t job_id = this->job_queue.front();
            Job& front_job = *this->jobs[job_id];

            while(front_job.hasTasks() && results.size() < max_jobs) {
                auto next_job = front_job.getNextTask();
                next_job.job = job_id;
                results.push_back(next_job);
            }

            if(!front_job.hasTasks()) {
                this->job_queue.pop_front();
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
        auto end = std::chrono::system_clock::now();
        log::write("Finished job ", job_id, " at ", std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count(), " ms");

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

    std::vector<JobStatus> JobQueue::getStatus() {
        std::vector<JobStatus> result;
        {
            std::unique_lock finished_jobs_lock(this->finished_mutex);

            for(uint32_t id : this->finished_jobs) {
                result.push_back(JobStatus{id, 0, 0});
            }
            this->finished_jobs.clear();
        }

        {
            std::shared_lock lock(this->job_map_mutex);

            for(auto& it : this->jobs) {
                uint32_t job_id = it.first;
                uint32_t total_frames = it.second->getTotalFrames();
                uint32_t frames_completed = it.second->getRenderedFrames();
                result.push_back(JobStatus{job_id, frames_completed, total_frames});
            }
        }
        return result;
    }
}