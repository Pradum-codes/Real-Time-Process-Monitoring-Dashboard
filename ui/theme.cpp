#include "ui/theme.h"

#include "imgui.h"

void apply_custom_style() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.CellPadding = ImVec2(10.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.34f, 0.52f, 0.85f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.23f, 0.42f, 0.62f, 0.95f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.14f, 0.30f, 0.46f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.30f, 0.45f, 0.90f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.40f, 0.58f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.24f, 0.38f, 1.00f);
}
