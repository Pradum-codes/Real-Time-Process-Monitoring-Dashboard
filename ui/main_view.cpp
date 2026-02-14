#include "ui/main_view.h"

#include "app/process_actions.h"
#include "imgui.h"
#include "ui/graphs_view.h"
#include "ui/process_table.h"
#include "ui/watchlist_tab.h"

void render_main_view(AppState& state) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("MainCanvas", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        ImGui::Text("Dashboard");
        ImGui::Separator();
        ImGui::Text("Processes: %zu", state.processes.size());
        ImGui::Separator();
        ImGui::Text("Refresh: %ds", state.refresh_seconds);
        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Processes")) {
            ImGui::BeginChild("ProcessControls", ImVec2(0, 96), true);
            ImGui::SetNextItemWidth(260.0f);
            ImGui::InputTextWithHint("##search", "Search process name or PID",
                                     state.search_query.data(), static_cast<int>(state.search_query.size()));

            ImGui::SameLine();
            ImGui::SetNextItemWidth(180.0f);
            ImGui::InputDouble("CPU Alert %", &state.cpu_threshold, 0.1, 1.0, "%.2f");

            ImGui::SameLine();
            ImGui::SetNextItemWidth(160.0f);
            ImGui::SliderInt("Refresh (s)", &state.refresh_seconds, 1, 10);

            const bool cpu_sort_active = (state.sort_mode == SortMode::CpuDesc);
            if (cpu_sort_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.23f, 0.42f, 0.62f, 1.00f));
            }
            if (ImGui::Button("Sort: CPU")) {
                state.sort_mode = SortMode::CpuDesc;
                apply_sort(state.processes, state.sort_mode);
            }
            if (cpu_sort_active) {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            const bool memory_sort_active = (state.sort_mode == SortMode::MemoryDesc);
            if (memory_sort_active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.23f, 0.42f, 0.62f, 1.00f));
            }
            if (ImGui::Button("Sort: Memory")) {
                state.sort_mode = SortMode::MemoryDesc;
                apply_sort(state.processes, state.sort_mode);
            }
            if (memory_sort_active) {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear Sort")) {
                state.sort_mode = SortMode::None;
            }
            ImGui::EndChild();
            ImGui::Spacing();
            render_process_list(state);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Graphs")) {
            render_graphs_view(state);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Watchlist")) {
            render_watchlist_tab(state);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
