#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H

#include <algorithm>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

// Struct to hold process information
struct ProcessInfo {
    int pid;
    std::string name;
};

// Function to check if a string is a number (helper)
bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

// Function to get process list
std::vector<ProcessInfo> get_process_list() {
    std::vector<ProcessInfo> processes;
    std::string proc_path = "/proc";

    for (const auto& entry : std::filesystem::directory_iterator(proc_path)) {
        std::string filename = entry.path().filename();

        if (is_number(filename)) {
            int pid = std::stoi(filename);
            std::ifstream cmd_file(proc_path + "/" + filename + "/comm");
            std::string process_name;
            std::getline(cmd_file, process_name);
            if (!process_name.empty()) {
                processes.push_back({pid, process_name});
            }
        }
    }
    return processes;
}

#endif // PROCESS_LIST_H
