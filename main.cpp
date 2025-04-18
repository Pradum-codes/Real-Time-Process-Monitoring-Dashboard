#include <iostream>
#include <chrono>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "process_list.h"
#include "system_metrics.h"
#include <signal.h>
#include <unistd.h>

// ---------------------- Error Callback ----------------------
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// ---------------------- Global Variables ----------------------
static int selected_pid = -1; // Selected Process ID
static ProcessInfo selected_process;
static char search_query[128] = ""; // Search query
double cpu_threshold = 0;

float cpu_usage = get_cpu_usage();
float memory_usage = get_memory_usage();

// ----------------------killing a proces -------------------------
bool kill_process(int pid) {
    // Check if process exists
    if (kill(pid, 0) == -1) {
        perror("Error: Process does not exist or insufficient permissions");
        return false;
    }

    std::cout << "Attempting to kill PID: " << pid << std::endl;

    // First try to terminate gracefully
    if (kill(pid, SIGTERM) == 0) {  
        std::cout << "Process terminated gracefully." << std::endl; 
        return true;
    }

    // If still running, force kill
    sleep(1); // Give some time for SIGTERM to take effect
    if (kill(pid, SIGKILL) == 0) {  
        std::cout << "Process forcefully killed." << std::endl; 
        return true;
    }

    perror("Error: Failed to kill process");
    return false;
}


// ---------------------- Render Process List ----------------------
void render_process_list(std::vector<ProcessInfo>& processes, double cpu_threshold) {

    // Setup Table with Columns
    if (ImGui::BeginTable("ProcessTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 700.0f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableSetupColumn("CPU Usage", ImGuiTableColumnFlags_WidthFixed, 300.0f);
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

            bool highlight = process.cpu_usage > cpu_threshold;
            ImVec4 row_color = highlight ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text);

            

                ImGui::TableSetColumnIndex(0);
                bool is_selected = (process.pid == selected_pid);
                if (highlight) ImGui::PushStyleColor(ImGuiCol_Text, row_color);
                if (ImGui::Selectable(std::to_string(process.pid).c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_pid = process.pid;
                    selected_process = process;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(process.name.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(process.state.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(process.memory.empty() ? "N/A" : process.memory.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f%%", process.cpu_usage);
            
                ImGui::TableSetColumnIndex(5);
                ImGui::TextUnformatted(process.threads.empty() ? "N/A" : process.threads.c_str());

                if (highlight) ImGui::PopStyleColor();
        }

        ImGui::EndTable();
    }
}


// ---------------------- Render Process Details ----------------------
void render_process_details(std::vector<ProcessInfo>& processes) {
    // Check if processes list is empty or no process is selected
    if (processes.empty() || selected_pid == -1) {
        ImGui::Text("No process selected or process list is empty.");
        ImGui::End();
        return;
    }

    if (selected_pid != -1) {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
        ImGui::SetNextWindowFocus();
        ImGui::Begin("Process Details", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Separator();
        ImGui::Text("Selected Process Details:");
        ImGui::Text("PID: %d", selected_process.pid);
        ImGui::Text("Name: %s", selected_process.name.c_str());
        ImGui::Text("State: %s", selected_process.state.c_str());
        ImGui::Text("Memory: %s", selected_process.memory.empty() ? "N/A" : selected_process.memory.c_str());
        ImGui::Text("Threads: %s", selected_process.threads.empty() ? "N/A" : selected_process.threads.c_str());

        ImGui::Separator();

        static bool show_popup = false;
        static auto popup_start_time = std::chrono::steady_clock::now();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 0.0f, 1.0f)); // Darker red when hovered
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button("Kill Process")) {
            if (kill_process(selected_process.pid)) {
                show_popup = true;
                popup_start_time = std::chrono::steady_clock::now();
            }
        }

        ImGui::PopStyleColor(3);

        // Show popup if active
        if (show_popup) {
            ImGui::SetNextWindowSize(ImVec2(300, 100));
            ImGui::Begin("Notification", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Process killed successfully!");

            // Check if 5 seconds have passed
            if (std::chrono::steady_clock::now() - popup_start_time > std::chrono::seconds(5)) {
                show_popup = false;  // Hide popup
            }

            ImGui::End();
        }


        ImGui::Separator();
        
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
        ImGui::InputDouble("Enter a CPU threshold", &cpu_threshold, 0.1, 1.0, "%.2f");
        
        // --- CPU and Memory Usage Bar Graph ---
        ImGui::Text("CPU Usage:");
        ImGui::Separator();
        float cpu_usage_normalized = cpu_usage / 100.0f; // Normalize CPU usage to [0, 1]
        ImVec2 bar_size = ImVec2(200, 30); // Width and height of the bar
        ImGui::ProgressBar(cpu_usage_normalized, bar_size, "");
        ImGui::SameLine();
        ImGui::Text("%.2f%%", cpu_usage);

        ImGui::Separator();

        ImGui::Text("Memory Usage:");
        ImGui::Separator();
        float memory_usage_normalized = memory_usage / 100.0f; // Normalize memory usage to [0, 1]
        ImVec2 memory_bar_size = ImVec2(200, 30); // Width and height of the bar
        ImGui::ProgressBar(memory_usage_normalized, memory_bar_size, "");
        ImGui::SameLine();
        ImGui::Text("%.2f%%", memory_usage);
        ImGui::Separator();

        if (ImGui::Button("Sort by CPU Usage")) {
            std::sort(processes.begin(), processes.end(), 
                [](const ProcessInfo& a, const ProcessInfo& b) {
                    return a.cpu_usage > b.cpu_usage; // Sort in descending order
                });
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Sort by Memory Usage")) {
            std::sort(processes.begin(), processes.end(), 
                [](const ProcessInfo& a, const ProcessInfo& b) {
                    // Handle non-numeric or empty memory values
                    long memory_a = a.memory.empty() ? 0 : std::stol(a.memory);
                    long memory_b = b.memory.empty() ? 0 : std::stol(b.memory);
                    return memory_a > memory_b; // Sort in descending order
                });
        }        

        // --- Render List ---
        render_process_list(processes, cpu_threshold);
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
