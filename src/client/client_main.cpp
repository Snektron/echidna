#include "utils/threadpool.hpp"

#include <iostream>

int main() {
    echidna::utils::ThreadPool pool(8);

    std::atomic<size_t> result = 0;
    for(size_t i = 0; i < 100; ++i) {
        pool.schedule([&result, i] {
            result += i;
        });
    }

    pool.stop();
    pool.join();

    std::cout << "Sum: " << result << std::endl;

    return 0;
}