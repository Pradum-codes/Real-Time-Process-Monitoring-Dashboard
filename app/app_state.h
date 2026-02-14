#pragma once

#include <array>
#include <vector>

#include "process_list.h"

enum class SortMode {
    None,
    CpuDesc,
    MemoryDesc
};

struct AppState {
    int selected_pid = -1;
    ProcessInfo selected_process{};
    bool process_details_popup_open = false;
    std::vector<int> watchlist_pids;
    std::array<char, 128> search_query{};
    double cpu_threshold = 0.0;
    float cpu_usage = 0.0f;
    float memory_usage = 0.0f;
    float swap_usage = 0.0f;
    float network_rx_kbps = 0.0f;
    float network_tx_kbps = 0.0f;
    int refresh_seconds = 2;
    std::vector<ProcessInfo> processes;
    std::vector<float> cpu_history;
    std::vector<float> memory_history;
    std::vector<float> swap_history;
    std::vector<float> network_rx_history;
    std::vector<float> network_tx_history;
    std::vector<float> per_cpu_usage;
    std::vector<std::vector<float>> per_cpu_histories;
    SortMode sort_mode = SortMode::None;
};

constexpr int kMaxHistoryPoints = 120;
