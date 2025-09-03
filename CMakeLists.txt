cmake_minimum_required(VERSION 3.10)
project(RealTimeProcessMonitoringDashboard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Sources (your code only)
# -----------------------------
set(SOURCES
    main.cpp
    process_list.h
    system_metrics.h
)

# -----------------------------
# ImGui sources
# -----------------------------
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Glad loader
set(GLAD_SOURCES external/glad/src/glad.c)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${IMGUI_SOURCES}
    ${GLAD_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    external/imgui
    external/imgui/backends
    external/glad/include
)

# Find and link GLFW
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES})

# Linux system libs
target_link_libraries(${PROJECT_NAME} PRIVATE
    GL
    dl
    pthread
    X11
)
