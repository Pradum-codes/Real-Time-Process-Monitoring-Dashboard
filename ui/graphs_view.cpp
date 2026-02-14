#include "ui/graphs_view.h"

#include <algorithm>
#include <string>
#include <vector>

#include "imgui.h"

namespace {

ImU32 make_series_color(size_t index, size_t count) {
    const float hue = (count == 0) ? 0.0f : static_cast<float>(index) / static_cast<float>(count);
    return ImColor::HSV(hue, 0.65f, 0.95f);
}

void render_multi_series_plot(const char* id,
                              const std::vector<const std::vector<float>*>& series,
                              const std::vector<ImU32>& colors,
                              float y_min,
                              float y_max,
                              float height) {
    ImVec2 plot_size(ImGui::GetContentRegionAvail().x, height);
    if (plot_size.x < 80.0f) {
        plot_size.x = 80.0f;
    }

    const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, plot_size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 canvas_max(canvas_pos.x + plot_size.x, canvas_pos.y + plot_size.y);
    draw_list->AddRectFilled(canvas_pos, canvas_max, ImGui::GetColorU32(ImGuiCol_FrameBg), 4.0f);
    draw_list->AddRect(canvas_pos, canvas_max, ImGui::GetColorU32(ImGuiCol_Border), 4.0f);

    const float left_pad = 12.0f;
    const float right_pad = 8.0f;
    const float top_pad = 8.0f;
    const float bottom_pad = 12.0f;
    const ImVec2 plot_min(canvas_pos.x + left_pad, canvas_pos.y + top_pad);
    const ImVec2 plot_max(canvas_max.x - right_pad, canvas_max.y - bottom_pad);
    const float plot_width = std::max(1.0f, plot_max.x - plot_min.x);
    const float plot_height = std::max(1.0f, plot_max.y - plot_min.y);
    const float range = std::max(1.0f, y_max - y_min);

    const ImU32 grid_color = ImGui::GetColorU32(ImVec4(0.70f, 0.70f, 0.70f, 0.20f));
    for (int i = 0; i <= 4; ++i) {
        const float t = static_cast<float>(i) / 4.0f;
        const float y = plot_min.y + t * plot_height;
        draw_list->AddLine(ImVec2(plot_min.x, y), ImVec2(plot_max.x, y), grid_color, 1.0f);
    }

    for (size_t series_idx = 0; series_idx < series.size(); ++series_idx) {
        const std::vector<float>& values = *series[series_idx];
        if (values.size() < 2) {
            continue;
        }

        const ImU32 line_color = colors[series_idx];
        const float x_scale = plot_width / static_cast<float>(values.size() - 1);
        ImVec2 prev;
        prev.x = plot_min.x;
        prev.y = plot_max.y - (std::clamp(values[0], y_min, y_max) - y_min) / range * plot_height;

        for (size_t i = 1; i < values.size(); ++i) {
            ImVec2 curr;
            curr.x = plot_min.x + static_cast<float>(i) * x_scale;
            curr.y = plot_max.y - (std::clamp(values[i], y_min, y_max) - y_min) / range * plot_height;
            draw_list->AddLine(prev, curr, line_color, 1.8f);
            prev = curr;
        }
    }
}

void render_series_legend(const std::vector<std::string>& labels, const std::vector<ImU32>& colors) {
    for (size_t i = 0; i < labels.size(); ++i) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(colors[i]));
        ImGui::TextUnformatted(labels[i].c_str());
        ImGui::PopStyleColor();
        if (i + 1 < labels.size()) {
            ImGui::SameLine();
            ImGui::TextUnformatted("|");
            ImGui::SameLine();
        }
    }
}

}  // namespace

void render_graphs_view(AppState& state) {
    ImGui::SetNextItemWidth(160.0f);
    ImGui::SliderInt("Refresh (s)", &state.refresh_seconds, 1, 10);
    ImGui::Spacing();

    ImGui::TextUnformatted("1. CPU");
    ImGui::Separator();
    if (state.per_cpu_histories.empty()) {
        ImGui::TextUnformatted("Waiting for CPU core data...");
    } else {
        std::vector<const std::vector<float>*> cpu_series;
        std::vector<std::string> cpu_labels;
        std::vector<ImU32> cpu_colors;
        cpu_series.reserve(state.per_cpu_histories.size());
        cpu_labels.reserve(state.per_cpu_histories.size());
        cpu_colors.reserve(state.per_cpu_histories.size());

        for (size_t i = 0; i < state.per_cpu_histories.size(); ++i) {
            cpu_series.push_back(&state.per_cpu_histories[i]);
            cpu_colors.push_back(make_series_color(i, state.per_cpu_histories.size()));
            const float current_usage =
                (i < state.per_cpu_usage.size()) ? state.per_cpu_usage[i] : 0.0f;
            cpu_labels.push_back("CPU " + std::to_string(i) + " " +
                                 std::to_string(static_cast<int>(current_usage)) + "%");
        }

        render_multi_series_plot("CpuCombinedPlot", cpu_series, cpu_colors, 0.0f, 100.0f, 200.0f);
        render_series_legend(cpu_labels, cpu_colors);
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("2. Memory and Swap");
    ImGui::Separator();
    std::vector<const std::vector<float>*> memory_series;
    std::vector<std::string> memory_labels;
    std::vector<ImU32> memory_colors;

    if (!state.memory_history.empty()) {
        memory_series.push_back(&state.memory_history);
        memory_labels.push_back("Memory " + std::to_string(static_cast<int>(state.memory_usage)) + "%");
        memory_colors.push_back(ImColor(100, 196, 255));
    }
    if (!state.swap_history.empty()) {
        memory_series.push_back(&state.swap_history);
        memory_labels.push_back("Swap " + std::to_string(static_cast<int>(state.swap_usage)) + "%");
        memory_colors.push_back(ImColor(255, 185, 82));
    }

    if (memory_series.empty()) {
        ImGui::TextUnformatted("Waiting for memory and swap data...");
    } else {
        render_multi_series_plot("MemorySwapCombinedPlot", memory_series, memory_colors, 0.0f, 100.0f, 170.0f);
        render_series_legend(memory_labels, memory_colors);
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("3. Network");
    ImGui::Separator();
    std::vector<const std::vector<float>*> network_series;
    std::vector<std::string> network_labels;
    std::vector<ImU32> network_colors;
    float network_max = 100.0f;

    if (!state.network_rx_history.empty()) {
        network_series.push_back(&state.network_rx_history);
        network_labels.push_back("RX " + std::to_string(static_cast<int>(state.network_rx_kbps)) + " KB/s");
        network_colors.push_back(ImColor(90, 230, 130));
        network_max = std::max(network_max,
                               *std::max_element(state.network_rx_history.begin(),
                                                 state.network_rx_history.end()) * 1.15f);
    }
    if (!state.network_tx_history.empty()) {
        network_series.push_back(&state.network_tx_history);
        network_labels.push_back("TX " + std::to_string(static_cast<int>(state.network_tx_kbps)) + " KB/s");
        network_colors.push_back(ImColor(255, 120, 120));
        network_max = std::max(network_max,
                               *std::max_element(state.network_tx_history.begin(),
                                                 state.network_tx_history.end()) * 1.15f);
    }

    if (network_series.empty()) {
        ImGui::TextUnformatted("Waiting for network data...");
    } else {
        render_multi_series_plot("NetworkCombinedPlot", network_series, network_colors, 0.0f, network_max, 170.0f);
        render_series_legend(network_labels, network_colors);
    }
}
