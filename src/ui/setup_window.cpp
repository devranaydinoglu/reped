#include <string>

#include "imgui.h"
#include "setup_window.h"
#include "../networking/server.h"
#include "../networking/client.h"
#include "../application.h"

SetupWindow::SetupWindow()
    : inputId(""), inputPort("8080"), inputAddress("localhost")
{
}

void SetupWindow::showSetupWindow(bool* open)
{
    IMGUI_CHECKVERSION();

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    // Start of the window
    ImGui::Begin("Setup", open, windowFlags);

    ImGuiStyle& style = ImGui::GetStyle();
    float labelWidth = 70.0f;
    float inputWidth = 156.0f;
    float totalWidth = labelWidth + style.ItemSpacing.x + inputWidth;
    float xCenter = (ImGui::GetWindowSize().x - totalWidth) * 0.5f;

    ImGui::SetCursorPosY(ImGui::GetWindowSize().y * 0.25f);

    // Shift the entire form block horizontally
    ImGui::SetCursorPosX(xCenter);

    // Create a child region wide enough to contain the form
    ImGui::BeginChild("FormRegion", ImVec2(totalWidth, 0), false);

    if (ImGui::BeginTable("InputTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, inputWidth);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Id");
        ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(inputWidth); ImGui::InputText("##Id", inputId, IM_ARRAYSIZE(inputId));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Port");
        ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(inputWidth); ImGui::InputText("##Port", inputPort, IM_ARRAYSIZE(inputPort));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Address");
        ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(inputWidth); ImGui::InputText("##Address", inputAddress, IM_ARRAYSIZE(inputAddress));

        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImVec2 size1 = ImGui::CalcTextSize("Create Server");
    ImVec2 size2 = ImGui::CalcTextSize("Connect As Client");
    float button1Width = size1.x + style.FramePadding.x * 2.0f;
    float button2Width = size2.x + style.FramePadding.x * 2.0f;
    float totalButtonWidth = button1Width + style.ItemSpacing.x + button2Width;

    float buttonX = (totalWidth - totalButtonWidth) * 0.5f;
    ImGui::SetCursorPosX(buttonX);

    if (ImGui::Button("Create Server"))
    {
        if (setupCompletedCallback)
            setupCompletedCallback(AppMode::SERVER, std::stoi(inputPort), inputAddress);
    }
    ImGui::SameLine();
    if (ImGui::Button("Connect As Client"))
    {
        if (setupCompletedCallback)
            setupCompletedCallback(AppMode::CLIENT, std::stoi(inputPort), inputAddress);
    }

    ImGui::EndChild();
    ImGui::End();
}
