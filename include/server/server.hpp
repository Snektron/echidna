#ifndef _ECHIDNA_SERVER_SERVER_HPP
#define _ECHIDNA_SERVER_SERVER_HPP

namespace echidna::server {
    class Server {
        private:

        public:
            Server();
            ~Server();

            void run();
            void join();
    };
}

#endif
