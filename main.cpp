#include <iostream>
#include <chrono>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "process_list.h"
#include "system_metrics.h"

// ---------------------- Error Callback ----------------------
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// ---------------------- Global Variables ----------------------
static int selected_pid = -1; // Selected Process ID
static char search_query[128] = ""; // Search query

float cpu_usage = get_cpu_usage();
float memory_usage = get_memory_usage();


// ---------------------- Render Process List ----------------------
void render_process_list(std::vector<ProcessInfo>& processes) {

    // Setup Table with Columns
    if (ImGui::BeginTable("ProcessTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 700.0f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow(); // Header row

        // Process Rows
        for (const auto& process : processes) {
            // Apply Search Filter
            if (strlen(search_query) > 0) {
                std::string search_lower = search_query;
                std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
                std::string name_lower = process.name;
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                if (name_lower.find(search_lower) == std::string::npos &&
                    std::to_string(process.pid).find(search_lower) == std::string::npos) {
                    continue; // Skip non-matching
                }
            }

            ImGui::TableNextRow(); // Next row

            // Individual Columns
            ImGui::TableSetColumnIndex(0);
            bool is_selected = (process.pid == selected_pid);
            if (ImGui::Selectable(std::to_string(process.pid).c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_pid = process.pid;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(process.name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(process.state.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(process.memory.empty() ? "N/A" : process.memory.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(process.threads.empty() ? "N/A" : process.threads.c_str());
        }

        ImGui::EndTable();
    }
}


// ---------------------- Render Process Details ----------------------
void render_process_details(std::vector<ProcessInfo>& processes) {
    ImGui::Begin("Process Details");

    // Check if processes list is empty or no process is selected
    if (processes.empty() || selected_pid == -1) {
        ImGui::Text("No process selected or process list is empty.");
        ImGui::End();
        return;
    }

    // Find the selected process
    auto it = std::find_if(processes.begin(), processes.end(), [](const ProcessInfo& p) { return p.pid == selected_pid; });
    if (it != processes.end()) {
        ImGui::Separator();
        ImGui::Text("Selected Process Details:");
        ImGui::Text("PID: %d", it->pid);
        ImGui::Text("Name: %s", it->name.c_str());
        ImGui::Text("State: %s", it->state.c_str());
        ImGui::Text("Memory: %s", it->memory.empty() ? "N/A" : it->memory.c_str());
        ImGui::Text("Threads: %s", it->threads.empty() ? "N/A" : it->threads.c_str());
    } else {
        ImGui::Text("Selected process not found.");
    }

    // Close button
    if (ImGui::Button("Close")) {
        selected_pid = -1;
    }

    ImGui::End();
}

// ---------------------- Main ----------------------
int main() {
    // ---- Setup & Initialization ----
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Process Monitor Dashboard", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ---- ImGui Setup ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.0f;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // ---- Process Management ----
    static std::vector<ProcessInfo> processes;
    auto last_refresh_time = std::chrono::steady_clock::now();
    const auto refresh_interval = std::chrono::seconds(2);

    // ---------------------- Main Loop ----------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Handle input/events

        // --- ImGui Frame Start ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ---- Fullscreen Window ----
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Process List", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

        // --- Auto Refresh Process List ---
        auto now = std::chrono::steady_clock::now();
        if (now - last_refresh_time >= refresh_interval) {
            processes = get_process_list();
            // Update system metrics
            cpu_usage = get_cpu_usage();
            memory_usage = get_memory_usage();
            last_refresh_time = now;
        }

        // --- Search Box ---
        ImGui::InputText("Search", search_query, IM_ARRAYSIZE(search_query));
        ImGui::Separator();

        // Display at top
        ImGui::Text("CPU Usage: %.2f%%", cpu_usage);
        ImGui::Text("Memory Usage: %.2f%%", memory_usage);
        ImGui::Separator();

        // --- Render List ---
        render_process_list(processes);
        ImGui::End(); // End Process List Window

        // --- Render Details if Process Selected ---
        if (selected_pid != -1) {
            render_process_details(processes);
        }

        // --- Render Everything ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ---------------------- Cleanup ----------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
