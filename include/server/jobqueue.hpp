#ifndef _ECHIDNA_SERVER_JOBQUEUE_HPP
#define _ECHIDNA_SERVER_JOBQUEUE_HPP

#include <deque>

#include "server/job.hpp"

namespace echidna::server {
    class JobQueue {
        private:

        public:
            JobQueue();
            ~JobQueue();
    };
}

#endif
