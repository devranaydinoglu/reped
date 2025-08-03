#include "editor.h"
#include "imgui.h"

Editor::Editor()
{

}

void Editor::showEditor(bool *open)
{
    IMGUI_CHECKVERSION();

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    // Start of the window
    ImGui::Begin("Editor", open, windowFlags);

    float margin = 20.0f;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 padding = style.WindowPadding;
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 editorSize(windowSize.x - 2 * margin, windowSize.y - 2 * margin);
    ImVec2 offset(margin, margin);
    
    ImGui::SetCursorPos(offset);
    
    char editorTextInput[4096];
    ImGui::InputTextMultiline("##EditorText", editorTextInput, IM_ARRAYSIZE(editorTextInput), editorSize);

    ImGui::End();
}
