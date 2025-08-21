#include <string>

#include "imgui.h"
#include "../../lib/ImGuiFileDialog/ImGuiFileDialog.h"

#include "setup_window.h"
#include "../application.h"

SetupWindow::SetupWindow()
    : inputClientId(""), inputPort("8080"), inputAddress("localhost")
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
    float columnWidth = labelWidth + style.ItemSpacing.x + inputWidth;
    float dividerWidth = 1.0f;
    float columnMargin = 20.0f; // Margin between columns and divider
    float totalWidth = (columnWidth * 2) + dividerWidth + (columnMargin * 2);
    float buttonMargin = 10.0f;
    float fixedColumnHeight = 350.0f; // Fixed height for both columns
    
    // Center the layout horizontally and vertically
    float xStart = (ImGui::GetWindowSize().x - totalWidth) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetWindowSize().y * 0.25f);
    ImGui::SetCursorPosX(xStart);

    // Left column (Server)
    ImGui::BeginChild("ServerColumn", ImVec2(columnWidth, fixedColumnHeight), false);
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetCursorPosX((columnWidth - ImGui::CalcTextSize("Server").x) * 0.5f);
    ImGui::Text("Server");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if (ImGui::BeginTable("ServerTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, inputWidth);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Port");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ServerPort", inputPort, IM_ARRAYSIZE(inputPort));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Address");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ServerAddress", inputAddress, IM_ARRAYSIZE(inputAddress));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("File");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::Button("Open File Dialog", ImVec2(inputWidth, 0)))
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config);
        }

        ImGui::EndTable();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        }
        
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    
    // Display selected file
    if (!filePathName.empty()) {
        std::string filename = filePathName.substr(filePathName.find_last_of("/\\") + 1);
        ImGui::TextWrapped("Selected: %s", filename.c_str());
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
    }

    // Full width server button with a small margin on sides
    if (ImGui::Button("Create Server", ImVec2(columnWidth, 0)))
    {
        if (setupCompletedCallback)
            setupCompletedCallback(AppMode::SERVER, std::stoi(inputPort), inputAddress, "", filePathName);
    }

    ImGui::EndChild();
    
    // Draw the divider
    ImGui::SameLine(0, columnMargin);
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - ImGui::GetScrollY()),
        ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y),
        ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
        dividerWidth
    );
    
    // Right column (Client)
    ImGui::SameLine(0, columnMargin + 20.0f);
    ImGui::BeginChild("ClientColumn", ImVec2(columnWidth, fixedColumnHeight), false);
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetCursorPosX((columnWidth - ImGui::CalcTextSize("Client").x) * 0.5f);
    ImGui::Text("Client");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if (ImGui::BeginTable("ClientTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, inputWidth);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("ClientId");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ClientId", inputClientId, IM_ARRAYSIZE(inputClientId));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Port");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ClientPort", inputPort, IM_ARRAYSIZE(inputPort));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Address");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##ClientAddress", inputAddress, IM_ARRAYSIZE(inputAddress));

        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    
    // Full width client button with a small margin on sides
    if (ImGui::Button("Connect As Client", ImVec2(columnWidth, 0)))
    {
        if (setupCompletedCallback)
            setupCompletedCallback(AppMode::CLIENT, std::stoi(inputPort), inputAddress, std::string(inputClientId), "");
    }

    ImGui::EndChild();
    ImGui::End();
}
