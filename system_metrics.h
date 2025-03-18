#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>

float get_memory_usage() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long total = 0, available = 0;

    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        long value;
        std::string unit;
        iss >> key >> value >> unit;
        if (key == "MemTotal:") total = value;
        else if (key == "MemAvailable:") available = value;
        if (total && available) break;
    }

    if (total == 0) return 0.0f; // Avoid divide by zero
    float used_percent = 100.0f * (total - available) / total;
    return used_percent;
}

// Helper to store CPU times
struct CpuTimes {
    long long user, nice, system, idle, iowait, irq, softirq, steal;
};

// Parse CPU times from /proc/stat
CpuTimes get_cpu_times() {
    std::ifstream stat("/proc/stat");
    std::string cpu;
    CpuTimes times = {0};
    stat >> cpu >> times.user >> times.nice >> times.system >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal;
    return times;
}

// Calculate CPU usage between two samples
float get_cpu_usage() {
    static CpuTimes prev = get_cpu_times();
    CpuTimes curr = get_cpu_times();

    long long prev_idle = prev.idle + prev.iowait;
    long long curr_idle = curr.idle + curr.iowait;

    long long prev_non_idle = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;
    long long curr_non_idle = curr.user + curr.nice + curr.system + curr.irq + curr.softirq + curr.steal;

    long long total_prev = prev_idle + prev_non_idle;
    long long total_curr = curr_idle + curr_non_idle;

    long long total_diff = total_curr - total_prev;
    long long idle_diff = curr_idle - prev_idle;

    prev = curr; // Update for next call

    if (total_diff == 0) return 0.0f; // Avoid divide by zero
    float cpu_percentage = 100.0f * (total_diff - idle_diff) / total_diff;
    return cpu_percentage;
}

#endif
