#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <GLFW/glfw3.h>

#include "app/app_state.h"
#include "app/process_actions.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "system_metrics.h"
#include "ui/main_view.h"
#include "ui/theme.h"

void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    const int initial_width = 1360;
    const int initial_height = 860;
    GLFWwindow* window = glfwCreateWindow(initial_width, initial_height, "Process Monitor Dashboard", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Wayland doesn't support programmatic window positioning.
    const bool is_wayland = (std::getenv("WAYLAND_DISPLAY") != nullptr);
    if (!is_wayland) {
        GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
        if (primary_monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
            if (mode) {
                const int pos_x = (mode->width - initial_width) / 2;
                const int pos_y = (mode->height - initial_height) / 2;
                glfwSetWindowPos(window, std::max(0, pos_x), std::max(0, pos_y));
            }
        }
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    apply_custom_style();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImVec4 clear_color = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);

    AppState state{};
    state.cpu_usage = get_cpu_usage();
    state.per_cpu_usage = get_per_cpu_usage();
    state.per_cpu_histories.assign(state.per_cpu_usage.size(), {});
    const MemorySwapUsage initial_mem_swap = get_memory_swap_usage();
    state.memory_usage = initial_mem_swap.memory_percent;
    state.swap_usage = initial_mem_swap.swap_percent;
    const NetworkThroughput initial_network = get_network_throughput();
    state.network_rx_kbps = initial_network.rx_kbps;
    state.network_tx_kbps = initial_network.tx_kbps;

    auto last_refresh_time = std::chrono::steady_clock::now();
    constexpr double target_fps = 30.0;
    const auto target_frame_time = std::chrono::duration<double>(1.0 / target_fps);

    while (!glfwWindowShouldClose(window)) {
        const auto frame_start = std::chrono::steady_clock::now();

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.2);
            continue;
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const auto now = std::chrono::steady_clock::now();
        if (now - last_refresh_time >= std::chrono::seconds(state.refresh_seconds)) {
            state.processes = get_process_list();
            apply_sort(state.processes, state.sort_mode);
            state.cpu_usage = get_cpu_usage();
            state.per_cpu_usage = get_per_cpu_usage();
            if (state.per_cpu_histories.size() != state.per_cpu_usage.size()) {
                state.per_cpu_histories.assign(state.per_cpu_usage.size(), {});
            }
            const MemorySwapUsage mem_swap = get_memory_swap_usage();
            state.memory_usage = mem_swap.memory_percent;
            state.swap_usage = mem_swap.swap_percent;
            const NetworkThroughput network = get_network_throughput();
            state.network_rx_kbps = network.rx_kbps;
            state.network_tx_kbps = network.tx_kbps;

            append_history_point(state.cpu_history, state.cpu_usage, kMaxHistoryPoints);
            append_history_point(state.memory_history, state.memory_usage, kMaxHistoryPoints);
            append_history_point(state.swap_history, state.swap_usage, kMaxHistoryPoints);
            append_history_point(state.network_rx_history, state.network_rx_kbps, kMaxHistoryPoints);
            append_history_point(state.network_tx_history, state.network_tx_kbps, kMaxHistoryPoints);
            for (size_t i = 0; i < state.per_cpu_usage.size(); ++i) {
                append_history_point(state.per_cpu_histories[i], state.per_cpu_usage[i], kMaxHistoryPoints);
            }
            last_refresh_time = now;
        }

        render_main_view(state);

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        const auto frame_end = std::chrono::steady_clock::now();
        const auto frame_time = frame_end - frame_start;
        if (frame_time < target_frame_time) {
            std::this_thread::sleep_for(target_frame_time - frame_time);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
