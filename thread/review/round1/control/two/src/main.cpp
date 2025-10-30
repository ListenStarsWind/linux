#include "iostream"
#include "my_thread.hpp"

int main() {
    auto thread = myThread<void>::create([]() {
        for (int i = 0; i < 10; ++i) {
            std::cout << i << std::endl;
            // sleep(1);
        }
    });

    for (int i = 10; i < 30; ++i) {
        std::cout << i << std::endl;
        // sleep(1);
    }

    thread->join();
    
    return 0;
}