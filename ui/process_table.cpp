#include "ui/process_table.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <string>
#include <vector>

#include "app/process_actions.h"
#include "imgui.h"

namespace {
bool is_in_watchlist(const AppState& state, int pid) {
    return std::find(state.watchlist_pids.begin(), state.watchlist_pids.end(), pid) !=
           state.watchlist_pids.end();
}

void set_watchlist_checked(AppState& state, int pid, bool checked) {
    auto it = std::find(state.watchlist_pids.begin(), state.watchlist_pids.end(), pid);
    if (checked && it == state.watchlist_pids.end()) {
        state.watchlist_pids.push_back(pid);
    } else if (!checked && it != state.watchlist_pids.end()) {
        state.watchlist_pids.erase(it);
    }
}
}  // namespace

void render_process_list(AppState& state) {
    if (ImGui::BeginTable("ProcessTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                             ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
                                             ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Watch", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.6f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("CPU Usage", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableHeadersRow();

        auto render_row = [&](const ProcessInfo& process) {
            ImGui::TableNextRow();
            bool highlight = process.cpu_usage > state.cpu_threshold;
            ImVec4 row_color = highlight ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
                                         : ImGui::GetStyleColorVec4(ImGuiCol_Text);

            ImGui::TableSetColumnIndex(0);
            bool watched = is_in_watchlist(state, process.pid);
            std::string checkbox_id = "##watch_" + std::to_string(process.pid);
            if (ImGui::Checkbox(checkbox_id.c_str(), &watched)) {
                set_watchlist_checked(state, process.pid, watched);
            }

            ImGui::TableSetColumnIndex(1);
            if (highlight) ImGui::PushStyleColor(ImGuiCol_Text, row_color);
            ImGui::Text("%d", process.pid);

            ImGui::TableSetColumnIndex(2);
            std::string name_label = process.name + "##name_" + std::to_string(process.pid);
            if (ImGui::Selectable(name_label.c_str(), false)) {
                state.selected_pid = process.pid;
                state.selected_process = process;
                state.process_details_popup_open = true;
            }

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(process.state.c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(process.memory.empty() ? "N/A" : process.memory.c_str());
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.2f%%", process.cpu_usage);
            ImGui::TableSetColumnIndex(6);
            ImGui::TextUnformatted(process.threads.empty() ? "N/A" : process.threads.c_str());

            if (highlight) ImGui::PopStyleColor();
        };

        const bool has_search = state.search_query[0] != '\0';
        if (!has_search) {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(state.processes.size()));
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    render_row(state.processes[row]);
                }
            }
        } else {
            std::string search_lower = state.search_query.data();
            std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            std::vector<const ProcessInfo*> filtered_processes;
            filtered_processes.reserve(state.processes.size());
            for (const auto& process : state.processes) {
                std::string name_lower = process.name;
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (name_lower.find(search_lower) != std::string::npos ||
                    std::to_string(process.pid).find(search_lower) != std::string::npos) {
                    filtered_processes.push_back(&process);
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(filtered_processes.size()));
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    render_row(*filtered_processes[row]);
                }
            }
        }

        ImGui::EndTable();
    }

    static bool show_notice = false;
    static auto notice_start = std::chrono::steady_clock::now();

    if (state.process_details_popup_open) {
        ImGui::OpenPopup("Process Details");
    }

    if (ImGui::BeginPopupModal("Process Details", &state.process_details_popup_open,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        auto selected_it = std::find_if(state.processes.begin(), state.processes.end(),
                                        [&](const ProcessInfo& p) { return p.pid == state.selected_pid; });
        if (selected_it != state.processes.end()) {
            state.selected_process = *selected_it;
        }

        ImGui::Text("PID: %d", state.selected_pid);
        if (selected_it == state.processes.end()) {
            ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.35f, 1.0f), "Process is no longer running.");
        } else {
            ImGui::Text("Name: %s", state.selected_process.name.c_str());
            ImGui::Text("State: %s", state.selected_process.state.c_str());
            ImGui::Text("Memory: %s", state.selected_process.memory.empty() ? "N/A" : state.selected_process.memory.c_str());
            ImGui::Text("CPU Usage: %.2f%%", state.selected_process.cpu_usage);
            ImGui::Text("Threads: %s", state.selected_process.threads.empty() ? "N/A" : state.selected_process.threads.c_str());
        }

        ImGui::Separator();
        bool watched = is_in_watchlist(state, state.selected_pid);
        std::string watch_button_label = watched ? "Remove from Watchlist" : "Add to Watchlist";
        if (ImGui::Button(watch_button_label.c_str())) {
            set_watchlist_checked(state, state.selected_pid, !watched);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.15f, 0.15f, 1.0f));
        const bool can_kill = (selected_it != state.processes.end());
        if (!can_kill) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Kill Process") && can_kill) {
            if (kill_process(state.selected_pid)) {
                show_notice = true;
                notice_start = std::chrono::steady_clock::now();
            }
        }
        if (!can_kill) {
            ImGui::EndDisabled();
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            state.process_details_popup_open = false;
            ImGui::CloseCurrentPopup();
        }

        if (show_notice) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.30f, 0.90f, 0.40f, 1.0f), "Kill signal sent.");
            if (std::chrono::steady_clock::now() - notice_start > std::chrono::seconds(3)) {
                show_notice = false;
            }
        }

        ImGui::EndPopup();
    }
}
