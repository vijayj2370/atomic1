#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

enum class ServiceState {
    Uninitialized,
    Initializing,
    Ready
};

std::atomic<ServiceState> current_state{ ServiceState::Uninitialized };

void init_and_use_service(int thread_id) {
    ServiceState expected = ServiceState::Uninitialized;

    if (current_state.compare_exchange_strong(expected, ServiceState::Initializing)) {
        std::cout << "Thread " << thread_id << " won the race! Booting service...\n";

        // Simulate heavy setup work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Update state to Ready and wake up all waiting threads
        current_state.store(ServiceState::Ready);
        current_state.notify_all();

        std::cout << "Thread " << thread_id << " finished booting.\n";
    }
    else {
        // 'expected' now holds the actual value of current_state
        if (expected == ServiceState::Initializing) {
            std::cout << "Thread " << thread_id << " is waiting for setup to finish...\n";

            // Efficiently sleep until current_state is no longer Initializing
            current_state.wait(ServiceState::Initializing);
        }
    }

    // All threads only reach this line once current_state == ServiceState::Ready
    std::cout << "Thread " << thread_id << " is now safely using the service!\n";
}

int main() {
    std::vector<std::thread> workers;

    for (int i = 1; i <= 5; ++i) {
        workers.emplace_back(init_and_use_service, i);
    }

    for (auto& t : workers) {
        t.join();
    }

    return 0;
}
