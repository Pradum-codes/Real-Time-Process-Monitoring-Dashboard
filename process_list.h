#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>  //for sys_conf()
#include <iomanip>   //for std::setprecision

struct ProcessInfo {
    int pid;
    std::string name;
    std::string state;
    std::string memory;
    std::string threads;
    float cpu_usage = 0.0f;
};

inline bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

inline float calculate_cpu_usage(int pid, long system_uptime) {
    std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
    if (!stat_file) return 0.0f;

    std::string line;
    std::getline(stat_file, line);
    std::istringstream iss(line);

    std::string token;
    int field_count = 0;
    long utime = 0, stime = 0, starttime = 0;

    // Parse the /proc/[pid]/stat file
    while (iss >> token) {
        field_count++;
        if (field_count == 14) utime = std::stol(token); // User mode time
        else if (field_count == 15) stime = std::stol(token); // Kernel mode time
        else if (field_count == 22) starttime = std::stol(token); // Start time
    }

    
    
    // Get clock ticks per second
    long hertz = sysconf(_SC_CLK_TCK);
    // get the core_count
    int core_count = sysconf(_SC_NPROCESSORS_ONLN);
    
    // Calculate total time spent by the process (in seconds)
    float total_time_secs = (utime + stime) / static_cast<float>(hertz);
    
    // Calculate seconds the process has been running
    float process_uptime = system_uptime - (starttime / static_cast<float>(hertz));
    
    // Avoid division by zero or negative values (e.g., process just started or invalid values)
    if (process_uptime <= 0) return 0.0f;
    
    // Calculate CPU usage as a percentage
    float cpu_usage = 100.0f * (total_time_secs / process_uptime);
    return cpu_usage /= core_count;
}

inline std::vector<ProcessInfo> get_process_list() {
    std::vector<ProcessInfo> processes;

    // Read system uptime from /proc/uptime
    std::ifstream uptime_file("/proc/uptime");
    if (!uptime_file) return processes;

    std::string uptime_line;
    std::getline(uptime_file, uptime_line);
    long system_uptime = std::stol(uptime_line.substr(0, uptime_line.find(' ')));

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;
        std::string filename = entry.path().filename();
        if (!is_number(filename)) continue;

        int pid = std::stoi(filename);
        std::ifstream status_file(entry.path() / "status");
        if (!status_file) continue;

        ProcessInfo proc;
        proc.pid = pid;
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.rfind("Name:", 0) == 0) {
                proc.name = line.substr(6);
            } else if (line.rfind("State:", 0) == 0) {
                proc.state = line.substr(7);
            } else if (line.rfind("VmSize:", 0) == 0) {
                std::string memory_str = line.substr(8);
                // Parse memory value and convert from KB to MB
                std::istringstream memory_stream(memory_str);
                long memory_kb = 0;
                std::string unit;
                if (memory_stream >> memory_kb >> unit) {
                    double memory_mb = memory_kb / 1024.0;
                    std::ostringstream formatted_memory;
                    formatted_memory << std::fixed << std::setprecision(2) << memory_mb << " MB";
                    proc.memory = formatted_memory.str();
                } else {
                    proc.memory = "N/A";
                }
            } else if (line.rfind("Threads:", 0) == 0) {
                proc.threads = line.substr(8);
            }
        }

        // Calculate CPU usage for the process
        proc.cpu_usage = calculate_cpu_usage(pid, system_uptime);

        processes.push_back(proc);
    }
    return processes;
}
