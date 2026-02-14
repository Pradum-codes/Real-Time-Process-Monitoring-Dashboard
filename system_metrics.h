#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include <chrono>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

struct MemorySwapUsage {
    float memory_percent = 0.0f;
    float swap_percent = 0.0f;
};

struct NetworkThroughput {
    float rx_kbps = 0.0f;
    float tx_kbps = 0.0f;
};

inline MemorySwapUsage get_memory_swap_usage() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long mem_total = 0;
    long mem_available = 0;
    long swap_total = 0;
    long swap_free = 0;

    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        long value;
        std::string unit;
        iss >> key >> value >> unit;
        if (key == "MemTotal:") mem_total = value;
        else if (key == "MemAvailable:") mem_available = value;
        else if (key == "SwapTotal:") swap_total = value;
        else if (key == "SwapFree:") swap_free = value;

        if (mem_total && mem_available && swap_total && swap_free) break;
    }

    MemorySwapUsage usage{};
    if (mem_total > 0) {
        usage.memory_percent = 100.0f * (mem_total - mem_available) / mem_total;
    }
    if (swap_total > 0) {
        usage.swap_percent = 100.0f * (swap_total - swap_free) / swap_total;
    }

    return usage;
}

inline float get_memory_usage() {
    return get_memory_swap_usage().memory_percent;
}

// Helper to store CPU times
struct CpuTimes {
    long long user, nice, system, idle, iowait, irq, softirq, steal;
};

// Parse CPU times from /proc/stat
inline CpuTimes get_cpu_times() {
    std::ifstream stat("/proc/stat");
    std::string cpu;
    CpuTimes times = {0};
    stat >> cpu >> times.user >> times.nice >> times.system >> times.idle >> times.iowait >> times.irq >> times.softirq >> times.steal;
    return times;
}

// Calculate CPU usage between two samples
inline float get_cpu_usage() {
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

inline std::vector<CpuTimes> get_per_cpu_times() {
    std::ifstream stat("/proc/stat");
    std::string line;
    std::vector<CpuTimes> cpu_times;

    while (std::getline(stat, line)) {
        if (line.rfind("cpu", 0) != 0) {
            break;
        }
        if (line.size() < 4 || !std::isdigit(static_cast<unsigned char>(line[3]))) {
            continue;
        }

        std::istringstream iss(line);
        std::string label;
        CpuTimes times{0, 0, 0, 0, 0, 0, 0, 0};
        iss >> label >> times.user >> times.nice >> times.system >> times.idle >>
            times.iowait >> times.irq >> times.softirq >> times.steal;
        cpu_times.push_back(times);
    }

    return cpu_times;
}

inline std::vector<float> get_per_cpu_usage() {
    static std::vector<CpuTimes> prev = get_per_cpu_times();
    const std::vector<CpuTimes> curr = get_per_cpu_times();

    if (prev.size() != curr.size()) {
        prev = curr;
    }

    std::vector<float> usage(curr.size(), 0.0f);
    for (size_t i = 0; i < curr.size(); ++i) {
        const long long prev_idle = prev[i].idle + prev[i].iowait;
        const long long curr_idle = curr[i].idle + curr[i].iowait;

        const long long prev_non_idle =
            prev[i].user + prev[i].nice + prev[i].system + prev[i].irq + prev[i].softirq + prev[i].steal;
        const long long curr_non_idle =
            curr[i].user + curr[i].nice + curr[i].system + curr[i].irq + curr[i].softirq + curr[i].steal;

        const long long total_prev = prev_idle + prev_non_idle;
        const long long total_curr = curr_idle + curr_non_idle;

        const long long total_diff = total_curr - total_prev;
        const long long idle_diff = curr_idle - prev_idle;

        if (total_diff > 0) {
            usage[i] = 100.0f * (total_diff - idle_diff) / total_diff;
        }
    }

    prev = curr;
    return usage;
}

inline std::pair<unsigned long long, unsigned long long> get_total_network_bytes() {
    std::ifstream net_dev("/proc/net/dev");
    std::string line;

    // Skip headers
    std::getline(net_dev, line);
    std::getline(net_dev, line);

    unsigned long long total_rx_bytes = 0;
    unsigned long long total_tx_bytes = 0;

    while (std::getline(net_dev, line)) {
        if (line.empty()) continue;

        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream iss(line);

        std::string iface;
        unsigned long long rx_bytes = 0;
        unsigned long long rx_packets = 0;
        unsigned long long rx_errs = 0;
        unsigned long long rx_drop = 0;
        unsigned long long rx_fifo = 0;
        unsigned long long rx_frame = 0;
        unsigned long long rx_compressed = 0;
        unsigned long long rx_multicast = 0;
        unsigned long long tx_bytes = 0;

        iss >> iface >> rx_bytes >> rx_packets >> rx_errs >> rx_drop >> rx_fifo >>
            rx_frame >> rx_compressed >> rx_multicast >> tx_bytes;

        if (iface == "lo") {
            continue;
        }

        total_rx_bytes += rx_bytes;
        total_tx_bytes += tx_bytes;
    }

    return {total_rx_bytes, total_tx_bytes};
}

inline NetworkThroughput get_network_throughput() {
    static auto prev_time = std::chrono::steady_clock::now();
    static auto prev_bytes = get_total_network_bytes();

    const auto now = std::chrono::steady_clock::now();
    const auto curr_bytes = get_total_network_bytes();
    const std::chrono::duration<double> elapsed = now - prev_time;

    NetworkThroughput throughput{};
    if (elapsed.count() > 0.0) {
        const double rx_bytes_per_sec = static_cast<double>(curr_bytes.first - prev_bytes.first) / elapsed.count();
        const double tx_bytes_per_sec = static_cast<double>(curr_bytes.second - prev_bytes.second) / elapsed.count();
        throughput.rx_kbps = static_cast<float>(rx_bytes_per_sec / 1024.0);
        throughput.tx_kbps = static_cast<float>(tx_bytes_per_sec / 1024.0);
    }

    prev_time = now;
    prev_bytes = curr_bytes;
    return throughput;
}

#endif
