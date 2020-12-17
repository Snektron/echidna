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
    class JobQueue {
        private:
            std::deque<Task> task_queue;
            std::deque<uint32_t> job_queue;
            std::vector<uint32_t> finished_jobs;

            uint32_t job_offset;
            std::unordered_map<uint32_t, Job*> jobs;

            std::mutex job_mutex;
            std::shared_mutex job_map_mutex;
            std::shared_mutex finished_mutex;
            std::condition_variable job_wait;
        public:
            JobQueue();
            ~JobQueue();

            void addJob(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);
            void addTasks(const std::vector<Task>&);
            std::vector<Task> getJobs(size_t);
            Job* findJob(uint32_t);
            void finishJob(uint32_t);
    };
}

#endif
