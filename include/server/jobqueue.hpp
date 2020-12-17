#ifndef _ECHIDNA_SERVER_JOBQUEUE_HPP
#define _ECHIDNA_SERVER_JOBQUEUE_HPP

#include <deque>
#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>

#include "server/job.hpp"
#include "server/task.hpp"

namespace echidna::server {
    class JobQueue {
        private:
            std::deque<Task> task_queue;
            std::deque<uint32_t> job_queue;

            uint32_t job_offset;
            std::unordered_map<uint32_t, Job*> jobs;

            std::mutex job_mutex;
            std::condition_variable job_wait;
        public:
            JobQueue();
            ~JobQueue();

            void addJob(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);
            void addTasks(const std::vector<Task>&);
            std::vector<Task> getJobs(size_t);
    };
}

#endif
