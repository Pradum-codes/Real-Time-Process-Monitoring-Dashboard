# Real-Time-Process-Monitoring-Dashboard
 Real-Time Process Monitoring Dashboard Description: Create a graphical dashboard that displays real-time information about process states, CPU usage, and memory consumption. The tool should allow administrators to manage processes efficiently and identify potential issues promptly

## 1. Project Overview
Goals:
- Develop a real-time graphical dashboard to monitor system processes.
- Track CPU usage, memory consumption, and process states dynamically.
- Provide administrators with process management capabilities (e.g., terminate, prioritize).
- Offer visual insights to identify performance bottlenecks.
Expected Outcomes:
- A functional, interactive dashboard that updates in real-time.
- Efficient data visualization of system resource usage.
- Administrator-level process management tools.
- A lightweight and responsive application with minimal system overhead.
```Scope:```
- Support for Linux-based systems (initially targeting Fedora, but adaptable).
- Use native system commands (e.g., ps, top, htop, procfs) for data collection.
- Ensure low latency in data updates (e.g., refresh every 1 second).

```Possible future enhancements (e.g., alerts for high CPU/memory usage, logging).```

## 2. Module-Wise Breakdown
The project can be divided into three key modules:
 1. Data Collection Module (Backend)
  Purpose: Gather real-time system information about processes, CPU usage, and memory consumption.
 2. Visualization & GUI Module (Frontend)
  Purpose: Display the collected data in an interactive, real-time dashboard.
 3. Process Management Module
  Purpose: Enable administrators to control processes (terminate, adjust priority, etc.).

## 3. Functionalities
### 1. Data Collection Module (Backend)
- Retrieve Process List → Fetch active processes, their PIDs, names, statuses (Running, Sleeping, etc.).
- CPU & Memory Usage Monitoring → Track system resource usage per process.
- Real-time Updates → Refresh data at 1-second intervals.
- Process Details → Display parent-child relationships, process command-line arguments.
- Implementation Example: Use C++ with /proc filesystem and system commands (ps, top, htop).
### 2. Visualization & GUI Module (Frontend)
- Dashboard Layout → Table-based view for process details.
- Graphical Charts → CPU and memory consumption visualized using real-time graphs.
- Sorting & Filtering → Allow sorting by CPU, memory usage, and filtering by process name.
- Implementation Example: Use Qt (C++ GUI framework) or React.js (web-based UI).
### 3. Process Management Module
- Kill Process → Terminate selected processes.
- Change Process Priority → Adjust priority (nice value).
- Start/Restart Process → Launch or restart a process.
- Alerts for High Resource Usage → Notify when CPU/memory crosses a threshold.
- Implementation Example: Use system calls (kill, nice, renice) and polkit for permissions.
## 4. Technology Recommendations
- Module	Technology Choices	Reasoning
- Backend (Data Collection)	C++, Linux system commands (ps, /proc API)	Low-level access to system processes, efficient memory usage.
- Frontend (GUI)	Qt (C++) OR React.js with Electron	Qt for native GUI; React + Electron for cross-platform flexibility.
- Data Visualization	Qt Charts (C++) OR Recharts/D3.js (JavaScript)	Qt Charts for native GUI, Recharts for web-based visualization.
- Process Management	Linux system calls (kill, nice)	Direct system interaction for process control.
## 5. Execution Plan
#### Phase 1: Setting Up the Project (Week 1-2)
✅ Choose whether to build a desktop app (Qt) or web-based app (React + Electron).<br>
✅ Set up basic project structure and integrate real-time data fetching from /proc.<br>
✅ Develop a command-line prototype for data collection.<br>

#### Phase 2: Implementing Core Features (Week 3-5)
✅ Build GUI framework with process table and resource usage graphs.<br>
✅ Implement real-time data updates (every second).<br>
✅ Ensure smooth performance (optimize queries from /proc).<br>

#### Phase 3: Process Management & Enhancements (Week 6-7)
✅ Add Kill Process, Change Priority functions.<br>
✅ Implement sorting, filtering, and alerts for high CPU/memory usage.<br>
✅ Fine-tune UI for better user experience.<br>

#### Phase 4: Testing & Deployment (Week 8)
✅ Conduct stress testing with heavy CPU load.<br>
✅ Optimize for low resource consumption.<br>
✅ Package for deployment (AppImage for Linux, or Electron for cross-platform).<br>

Compilation Code
```bash
g++ main.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp glad/glad.c -I. -lglfw -ldl -lGL -std=c++17 -o dashboard
```
📂 Repository Structure<br>
.<br>
├── main.cpp                 # Application entry point<br>
├── process_list.h           # Logic for retrieving process information<br>
├── system_metrics.h         # Logic for system metrics (CPU, memory, etc.)<br>
│<br>
├── glad/                    # Glad OpenGL loader (third-party)<br>
│<br>
├── imgui.cpp                # Dear ImGui core (third-party)<br>
├── imgui.h<br>
├── imgui_draw.cpp<br>
├── imgui_widgets.cpp<br>
├── imgui_tables.cpp<br>
├── imgui_internal.h<br>
├── imgui_demo.cpp           # ImGui demo window (optional)<br>
├── imconfig.h<br>
├── imstb_rectpack.h         # ImGui stb headers<br>
├── imstb_textedit.h<br>
├── imstb_truetype.h<br>
│<br>
├── imgui_impl_glfw.cpp      # ImGui backend for GLFW<br>
├── imgui_impl_glfw.h<br>
├── imgui_impl_opengl3.cpp   # ImGui backend for OpenGL3<br>
├── imgui_impl_opengl3.h<br>
│<br>
├── dashboard                # Compiled binary (ignored in clean builds)<br>
├── imgui.ini                # ImGui runtime settings<br>
└── README.md<br>

### Note:
The repo currently includes full ImGui and Glad sources. Long-term, these should ideally be pulled in as Git submodules instead of committing the entire third-party code.

## 🛠️ Build Instructions
### Requirements
- C++17 compiler (GCC/Clang/MSVC)
- CMake (>= 3.10 recommended)
- GLFW development package

```bash
sudo dnf install glfw-devel
```

### Build
```bash
git clone <this-repo-url>
cd Real-Time-Process-Monitoring-Dashboard
mkdir build && cd build
cmake ..
make
```


Run with:

./dashboard

