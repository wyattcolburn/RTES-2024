#include <iostream>
#include <thread>

void thread_function(int thread_id) {
    std::cout << "Thread " << thread_id << ": Hello from thread!" << std::endl;
}

int main() {
    std::thread thread1(thread_function, 1);
    std::thread thread2(thread_function, 2);

    // Wait for both threads to finish
    thread1.join();
    thread2.join();

    return 0;
}

