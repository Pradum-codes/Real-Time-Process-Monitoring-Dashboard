#include "app/process_actions.h"

#include <algorithm>
#include <iostream>
#include <string>

#include <signal.h>
#include <unistd.h>

bool kill_process(int pid) {
    if (kill(pid, 0) == -1) {
        perror("Error: Process does not exist or insufficient permissions");
        return false;
    }

    std::cout << "Attempting to kill PID: " << pid << std::endl;

    if (kill(pid, SIGTERM) == 0) {
        std::cout << "Process terminated gracefully." << std::endl;
        return true;
    }

    sleep(1);
    if (kill(pid, SIGKILL) == 0) {
        std::cout << "Process forcefully killed." << std::endl;
        return true;
    }

    perror("Error: Failed to kill process");
    return false;
}

void apply_sort(std::vector<ProcessInfo>& data, SortMode sort_mode) {
    if (sort_mode == SortMode::CpuDesc) {
        std::sort(data.begin(), data.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                      return a.cpu_usage > b.cpu_usage;
                  });
    } else if (sort_mode == SortMode::MemoryDesc) {
        std::sort(data.begin(), data.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                      long memory_a = a.memory.empty() ? 0 : std::stol(a.memory);
                      long memory_b = b.memory.empty() ? 0 : std::stol(b.memory);
                      return memory_a > memory_b;
                  });
    }
}

void append_history_point(std::vector<float>& history, float value, int max_points) {
    history.push_back(value);
    if (static_cast<int>(history.size()) > max_points) {
        history.erase(history.begin(), history.begin() + (history.size() - max_points));
    }
}
