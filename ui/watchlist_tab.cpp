#include "ui/watchlist_tab.h"

#include <algorithm>
#include <chrono>
#include <string>

#include "app/process_actions.h"
#include "imgui.h"

void render_watchlist_tab(AppState& state) {
    if (state.watchlist_pids.empty()) {
        ImGui::Text("No process in watchlist. Use the checkboxes in the Processes tab.");
        return;
    }

    std::vector<int> pids_to_remove;
    static bool show_notice = false;
    static auto notice_start = std::chrono::steady_clock::now();

    const float spacing = 12.0f;
    const float card_width = 320.0f;
    const float available_width = ImGui::GetContentRegionAvail().x;
    int columns = static_cast<int>((available_width + spacing) / (card_width + spacing));
    if (columns < 1) {
        columns = 1;
    }

    int index = 0;
    for (int pid : state.watchlist_pids) {
        auto it = std::find_if(state.processes.begin(), state.processes.end(),
                               [&](const ProcessInfo& p) { return p.pid == pid; });
        const bool available = (it != state.processes.end());
        const ProcessInfo* process = available ? &(*it) : nullptr;

        ImGui::PushID(pid);
        ImGui::BeginChild("WatchCard", ImVec2(card_width, 170.0f), true);

        ImGui::Text("PID: %d", pid);
        ImGui::Text("Name: %s", available ? process->name.c_str() : "N/A");
        ImGui::Text("State: %s", available ? process->state.c_str() : "N/A");
        ImGui::Text("Memory: %s", available && !process->memory.empty() ? process->memory.c_str() : "N/A");
        if (available) {
            ImGui::Text("CPU: %.2f%%", process->cpu_usage);
        } else {
            ImGui::Text("CPU: N/A");
        }
        ImGui::Text("Threads: %s", available && !process->threads.empty() ? process->threads.c_str() : "N/A");

        ImGui::Spacing();
        if (available) {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "Running");
        } else {
            ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.35f, 1.0f), "Exited");
        }

        if (!available) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Kill") && available) {
            if (kill_process(pid)) {
                show_notice = true;
                notice_start = std::chrono::steady_clock::now();
            }
        }
        if (!available) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            pids_to_remove.push_back(pid);
        }

        ImGui::EndChild();
        ImGui::PopID();

        ++index;
        if ((index % columns) != 0) {
            ImGui::SameLine(0.0f, spacing);
        }
    }

    if (!pids_to_remove.empty()) {
        state.watchlist_pids.erase(
            std::remove_if(state.watchlist_pids.begin(), state.watchlist_pids.end(),
                           [&](int pid) {
                               return std::find(pids_to_remove.begin(), pids_to_remove.end(), pid) !=
                                      pids_to_remove.end();
                           }),
            state.watchlist_pids.end());
    }

    if (show_notice) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.30f, 0.90f, 0.40f, 1.0f), "Process killed successfully.");
        if (std::chrono::steady_clock::now() - notice_start > std::chrono::seconds(3)) {
            show_notice = false;
        }
    }
}
