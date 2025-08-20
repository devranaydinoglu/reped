#include <algorithm>
#include <SDL3/SDL.h>

#include "editor.h"
#include "imgui.h"
#include "../controller/controller.h"
#include "../text_engine/input_events.h"

Editor::Editor()
    : controller(nullptr), cursorLastMovedTime(0.0f)
{
}

void Editor::showEditor(bool* open)
{
    IMGUI_CHECKVERSION();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 editorSize(io.DisplaySize.x, io.DisplaySize.y);
    ImGui::SetNextWindowSize(editorSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Editor", open, windowFlags);

    // Draw background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentAreaOrigin = ImGui::GetCursorScreenPos();
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
    ImVec2 pMax = ImVec2(contentAreaOrigin.x + contentRegionAvail.x, contentAreaOrigin.y + contentRegionAvail.y);
    drawList->AddRectFilled(contentAreaOrigin, pMax, IM_COL32(30, 30, 30, 255));

    // Input capture
    ImGui::SetCursorScreenPos(contentAreaOrigin);
    ImGui::InvisibleButton("EditorInputCapture", contentRegionAvail, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::SetItemDefaultFocus();
    ImGui::SetKeyboardFocusHere();

    bool editorIsActive = ImGui::IsItemFocused();
    ImFont* font = ImGui::GetFont();
    float lineHeight = ImGui::GetTextLineHeight();
    float charWidth = font->CalcTextSizeA(font->LegacySize, FLT_MAX, 0.0f, "A").x;

    float baseX = contentAreaOrigin.x + 5.0f;
    float baseY = contentAreaOrigin.y + 5.0f;

    std::string text = controller->getText();
    size_t cursorPos = controller->getCursorPosition();
    
    // INPUT HANDLING
    if (editorIsActive)
    {
        // Text input is handled via SDL events in handleTextInput()
        // Only handle special keys here
        size_t cursorPos = controller->getCursorPosition();
        std::string text = controller->getText();

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && cursorPos > 0)
        {
            controller->handleCursorInputEvent(CursorInputEvent(cursorPos - 1));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && cursorPos < text.size())
        {
            controller->handleCursorInputEvent(CursorInputEvent(cursorPos + 1));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }

        auto it = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), cursorPos);
        size_t currentLine = std::distance(lineStartOffsets.begin(), it) - 1;
        size_t column = cursorPos - lineStartOffsets[currentLine];

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && currentLine > 0)
        {
            size_t prevLineStart = lineStartOffsets[currentLine - 1];
            size_t prevLineEnd = lineStartOffsets[currentLine];
            controller->handleCursorInputEvent(CursorInputEvent(prevLineStart + std::min(column, prevLineEnd - prevLineStart - 1)));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && currentLine + 1 < lineStartOffsets.size() - 1)
        {
            size_t nextLineStart = lineStartOffsets[currentLine + 1];
            size_t nextLineEnd = lineStartOffsets[currentLine + 2];
            controller->handleCursorInputEvent(CursorInputEvent(nextLineStart + std::min(column, nextLineEnd - nextLineStart - 1)));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && cursorPos > 0)
        {
            controller->handleTextInputEvent(TextInputEvent(TextInputEventType::DELETE, "\0", cursorPos - 1));
            controller->handleCursorInputEvent(CursorInputEvent(cursorPos - 1));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            controller->handleTextInputEvent(TextInputEvent(TextInputEventType::INSERT, "\n", cursorPos));
            controller->handleCursorInputEvent(CursorInputEvent(cursorPos + 1));
            cursorPos = controller->getCursorPosition();
            cursorLastMovedTime = ImGui::GetTime();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
        {
            std::size_t charPos = mousePosToCharPos(baseX, baseY, charWidth, lineHeight, text.size());
            controller->handleCursorInputEvent(CursorInputEvent(charPos));

            cursorLastMovedTime = ImGui::GetTime();
        }
    }

    cursorPos = controller->getCursorPosition();

    // Re-fetch updated text after input handling
    text = controller->getText();
    
    // Build lineStartOffsets after input changed text
    lineStartOffsets.clear();
    lineStartOffsets.emplace_back(0);
    for (size_t i = 0; i < text.size(); i++)
        if (text[i] == '\n')
            lineStartOffsets.emplace_back(i + 1);
    if (lineStartOffsets.empty())
        lineStartOffsets.emplace_back(text.size());

    // RENDERING
    text = controller->getText();
    
    size_t numCharsRendered = 0;

    for (size_t lineIndex = 0; lineIndex <= lineStartOffsets.size() - 1; ++lineIndex)
    {
        size_t lineStart = lineStartOffsets[lineIndex];
        size_t lineEnd = lineStartOffsets[lineIndex + 1];
        std::size_t len;
        if (lineEnd == 0)
            len = text.size();
        else
            len = lineEnd;
        std::string lineBuffer = text.substr(lineStart, len);
        ImVec2 linePos = ImVec2(baseX, baseY + lineIndex * lineHeight);
        drawList->AddText(linePos, IM_COL32_WHITE, lineBuffer.c_str());
        numCharsRendered += lineBuffer.size();
    }

    // Cursor line/column
    auto it = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), cursorPos);
    size_t cursorLine = std::distance(lineStartOffsets.begin(), it) - 1;
    size_t cursorColumn = cursorPos - lineStartOffsets[cursorLine];

    float visualCursorX = baseX + cursorColumn * charWidth;
    float visualCursorY = baseY + cursorLine * lineHeight;

    // Draw blinking cursor
    float timeSinceMove = ImGui::GetTime() - cursorLastMovedTime;
    bool showCursor = (timeSinceMove < 0.5f) || (static_cast<int>((ImGui::GetTime() * 2.0f)) % 2 == 0);

    if (showCursor)
    {
        drawList->AddLine(
            ImVec2(visualCursorX, visualCursorY),
            ImVec2(visualCursorX, visualCursorY + lineHeight),
            IM_COL32_WHITE, 1.0f);
    }

    ImGui::End();
}

void Editor::handleTextInput(const char* text)
{
    if (!controller || !text)
        return;
    
    std::string inputText(text);
    if (!inputText.empty())
    {
        std::size_t cursorPos = controller->getCursorPosition();
        controller->handleTextInputEvent(TextInputEvent(TextInputEventType::INSERT, inputText, cursorPos, inputText.size()));
        controller->handleCursorInputEvent(CursorInputEvent(cursorPos + inputText.size()));
        cursorLastMovedTime = ImGui::GetTime();
    }
}

std::size_t Editor::mousePosToCharPos(float baseX, float baseY, float charWidth, float lineHeight, std::size_t textLength)
{
    std::size_t xIndex = (ImGui::GetMousePos().x - baseX) / charWidth;
    std::size_t yIndex = (ImGui::GetMousePos().y - baseY) / lineHeight;
    
    yIndex = std::min(yIndex, lineStartOffsets.size() - 1);

    std::size_t lineLength;
    if (yIndex + 1 < lineStartOffsets.size())
        lineLength = lineStartOffsets[yIndex + 1] - lineStartOffsets[yIndex] - 1;
    else
        lineLength = textLength - lineStartOffsets[yIndex];

    xIndex = std::min(xIndex, lineLength);
    
    return static_cast<std::size_t>(lineStartOffsets[yIndex] + xIndex);
}
