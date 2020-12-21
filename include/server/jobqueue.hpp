#ifndef _ECHIDNA_SERVER_JOBQUEUE_HPP
#define _ECHIDNA_SERVER_JOBQUEUE_HPP

#include <deque>
#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include "server/job.hpp"
#include "server/task.hpp"

namespace echidna::server {
    //Status of a single job
    struct JobStatus {
        uint32_t job_id;
        uint32_t frames_rendered;
        uint32_t frames_total;
    };

    //A queue managing the set of jobs to run
    class JobQueue {
        private:
            //Individual task queue, only used for priority jobs (e.g. jobs redistributed after a client crash)
            std::deque<Task> task_queue;
            //The job queue, contains a set of job IDs
            std::deque<uint32_t> job_queue;
            //The set of all currently finished jobs
            std::vector<uint32_t> finished_jobs;

            //Lookup table mapping job ids to job data
            uint32_t job_offset;
            std::unordered_map<uint32_t, Job*> jobs;

            //Threading and synchronisation
            std::mutex job_mutex;
            std::shared_mutex job_map_mutex;
            std::shared_mutex finished_mutex;
            std::condition_variable job_wait;
        public:
            JobQueue();
            ~JobQueue();

            //Creates a new job from the given parameters
            uint32_t addJob(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);
            //Adds a set of tasks to the priority queue
            void addTasks(const std::vector<Task>&);
            //Returns up the the requested number of jobs from the queue
            std::vector<Task> getJobs(size_t);
            //Finds the job description from a given job id
            Job* findJob(uint32_t);
            //Marks a job as finished
            void finishJob(uint32_t);

            //Convert the job queue into a status report
            std::vector<JobStatus> getStatus();
    };
}

#endif
