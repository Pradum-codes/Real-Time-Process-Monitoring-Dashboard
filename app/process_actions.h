#pragma once

#include <vector>

#include "app/app_state.h"

bool kill_process(int pid);
void apply_sort(std::vector<ProcessInfo>& data, SortMode sort_mode);
void append_history_point(std::vector<float>& history, float value, int max_points);
