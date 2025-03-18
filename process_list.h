#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

struct ProcessInfo {
    int pid;
    std::string name;
    std::string state;
    std::string memory;
    std::string threads;
};

bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

std::vector<ProcessInfo> get_process_list() {
    std::vector<ProcessInfo> processes;
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
                proc.memory = line.substr(8);
            } else if (line.rfind("Threads:", 0) == 0) { // Fixed typo here
                proc.threads = line.substr(8);
            }
        }
        processes.push_back(proc);
    }
    return processes;
}