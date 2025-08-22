#include <algorithm>
#include <SDL3/SDL.h>

#include "editor.h"
#include "imgui.h"
#include "../controller/controller.h"
#include "../text_engine/input_events.h"

Editor::Editor()
    : controller(nullptr), cursorLastMovedTime(0.0f), isDragging(false), selectionStartPos(0), selectionEndPos(0), lineScrollOffsetY(0)
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

    const float statusBarHeight = 20.0f;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentAreaOrigin = ImGui::GetCursorScreenPos();
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
    
    ImVec2 textAreaSize(contentRegionAvail.x, contentRegionAvail.y - statusBarHeight);
    
    // Draw background
    ImVec2 pMax = ImVec2(contentAreaOrigin.x + textAreaSize.x, contentAreaOrigin.y + textAreaSize.y);
    drawList->AddRectFilled(contentAreaOrigin, pMax, IM_COL32(30, 30, 30, 255));

    ImGui::SetCursorScreenPos(contentAreaOrigin);
    ImGui::InvisibleButton("EditorInputCapture", textAreaSize, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::SetItemDefaultFocus();
    ImGui::SetKeyboardFocusHere();

    bool editorIsActive = ImGui::IsItemFocused();
    ImFont* font = ImGui::GetFont();
    float lineHeight = ImGui::GetTextLineHeight();
    float charWidth = font->CalcTextSizeA(font->LegacySize, FLT_MAX, 0.0f, "A").x;

    float baseX = contentAreaOrigin.x + 5.0f;
    float baseY = contentAreaOrigin.y + 5.0f;

    std::string text = controller->getText();
    std::size_t cursorPos = controller->getCursorPosition();
    
    // INPUT HANDLING
    if (editorIsActive)
    {
        // Text input is handled via SDL events in handleTextInput()
        // Only special keys are handled here
        std::size_t cursorPos = controller->getCursorPosition();
        std::string text = controller->getText();

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && cursorPos > 0)
        {            
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            {
                // Start selection if none exists
                if (!hasSelection())
                {
                    selectionStartPos = cursorPos;
                    selectionEndPos = cursorPos;
                }
                
                // Move cursor and extend selection
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos - 1));
                cursorPos = controller->getCursorPosition();
                selectionEndPos = cursorPos;
                cursorLastMovedTime = ImGui::GetTime();
            }
            else
            {
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos - 1));
                cursorPos = controller->getCursorPosition();
                onCursorMoved();
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && cursorPos < text.size())
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            {
                if (!hasSelection())
                {
                    selectionStartPos = cursorPos;
                    selectionEndPos = cursorPos;
                }
                
                // Move cursor and extend selection
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos + 1));
                cursorPos = controller->getCursorPosition();
                selectionEndPos = cursorPos;
                cursorLastMovedTime = ImGui::GetTime();
            }
            else
            {
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos + 1));
                cursorPos = controller->getCursorPosition();
                onCursorMoved();
            }
        }

        auto it = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), cursorPos);
        std::size_t currentLine = std::distance(lineStartOffsets.begin(), it) - 1;
        std::size_t column = cursorPos - lineStartOffsets[currentLine];

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && currentLine > 0)
        {
            std::size_t prevLineStart = lineStartOffsets[currentLine - 1];
            std::size_t prevLineEnd = lineStartOffsets[currentLine];
            
            // Calculate the length of the previous line (excluding newline character)
            std::size_t prevLineLength = prevLineEnd - prevLineStart;
            if (prevLineLength > 0 && prevLineStart + prevLineLength - 1 < text.size() && text[prevLineStart + prevLineLength - 1] == '\n') {
                prevLineLength--; // Don't count the newline character
            }
            
            // Position cursor at the same column or at the end of the line if it's shorter
            std::size_t targetColumn = std::min(column, prevLineLength);
            
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            {
                if (!hasSelection())
                {
                    selectionStartPos = cursorPos;
                    selectionEndPos = cursorPos;
                }

                // Move cursor and extend selection
                controller->handleCursorInputEvent(CursorInputEvent(prevLineStart + targetColumn));
                cursorPos = controller->getCursorPosition();
                selectionEndPos = cursorPos;
                cursorLastMovedTime = ImGui::GetTime();
            }
            else
            {
                controller->handleCursorInputEvent(CursorInputEvent(prevLineStart + targetColumn));
                cursorPos = controller->getCursorPosition();
                onCursorMoved();
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && currentLine + 1 < lineStartOffsets.size())
        {
            std::size_t nextLineStart = lineStartOffsets[currentLine + 1];
            std::size_t nextLineEnd;
            
            if (currentLine + 2 < lineStartOffsets.size())
                nextLineEnd = lineStartOffsets[currentLine + 2];
            else
                nextLineEnd = text.size();
            
            // Calculate the length of the next line (excluding newline character)
            size_t nextLineLength = nextLineEnd - nextLineStart;
            if (nextLineLength > 0 && nextLineStart + nextLineLength - 1 < text.size() && text[nextLineStart + nextLineLength - 1] == '\n') {
                nextLineLength--; // Don't count the newline character
            }
            
            // Position cursor at the same column or at the end of the line if it's shorter
            size_t targetColumn = std::min(column, nextLineLength);
            
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            {
                if (!hasSelection())
                {
                    selectionStartPos = cursorPos;
                    selectionEndPos = cursorPos;
                }

                // Move cursor and extend selection
                controller->handleCursorInputEvent(CursorInputEvent(nextLineStart + targetColumn));
                cursorPos = controller->getCursorPosition();
                selectionEndPos = cursorPos;
                cursorLastMovedTime = ImGui::GetTime();
            }
            else
            {
                controller->handleCursorInputEvent(CursorInputEvent(nextLineStart + targetColumn));
                cursorPos = controller->getCursorPosition();
                onCursorMoved();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
        {
            if (hasSelection())
            {
                deleteSelectedText(cursorPos);
                cursorLastMovedTime = ImGui::GetTime();
            }
            else if (cursorPos > 0)
            {
                controller->handleTextInputEvent(TextInputEvent(TextInputEventType::DELETE, "\0", cursorPos - 1));
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos - 1));
                cursorPos = controller->getCursorPosition();
                onCursorMoved();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            controller->handleTextInputEvent(TextInputEvent(TextInputEventType::INSERT, "\n", cursorPos));
            controller->handleCursorInputEvent(CursorInputEvent(cursorPos + 1));
            cursorPos = controller->getCursorPosition();
            onCursorMoved();
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            std::size_t charPos = mousePosToCharPos(baseX, baseY, charWidth, lineHeight, text.size());
            controller->handleCursorInputEvent(CursorInputEvent(charPos));
            
            isDragging = true;
            selectionStartPos = charPos;
            selectionEndPos = charPos;

            cursorLastMovedTime = ImGui::GetTime();
        }
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && isDragging)
        {
            std::size_t charPos = mousePosToCharPos(baseX, baseY, charWidth, lineHeight, text.size());
            selectionEndPos = charPos;
            
            controller->handleCursorInputEvent(CursorInputEvent(charPos));
            cursorLastMovedTime = ImGui::GetTime();
        }
        
        // End dragging when mouse is released
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            isDragging = false;

        // Handle copy
        if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_C))
        {
            if (hasSelection())
            {
                std::size_t selStart = getSelectionStart();
                std::size_t selEnd = getSelectionEnd();
                std::string copyText = text.substr(selStart, selEnd - selStart);
                SDL_SetClipboardText(copyText.c_str());
            }
        }
        
        // Handle paste
        if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_V))
        {
            char* clipboardText = SDL_GetClipboardText();
            if (clipboardText && strlen(clipboardText) > 0)
            {
                std::string pasteText(clipboardText);
                
                if (hasSelection())
                    deleteSelectedText(cursorPos);
                
                controller->handleTextInputEvent(TextInputEvent(TextInputEventType::INSERT, pasteText, cursorPos, pasteText.size()));
                controller->handleCursorInputEvent(CursorInputEvent(cursorPos + pasteText.size()));
                onCursorMoved();
            }
            if (clipboardText)
            {
                SDL_free(clipboardText);
            }
        }
    }

    cursorPos = controller->getCursorPosition();

    // Re-fetch updated text after input handling
    text = controller->getText();
    
    // Build lineStartOffsets after input changed text
    lineStartOffsets.clear();
    lineStartOffsets.emplace_back(0);
    for (std::size_t i = 0; i < text.size(); i++)
        if (text[i] == '\n')
            lineStartOffsets.emplace_back(i + 1);
    if (lineStartOffsets.empty())
        lineStartOffsets.emplace_back(text.size());

    // Scrolling
    std::size_t maxRenderableLines = textAreaSize.y / lineHeight - 1;

    auto cursorLinePosIt = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), cursorPos);
    std::size_t cursorLinePos = std::distance(lineStartOffsets.begin(), cursorLinePosIt) - 1;
    
    if (cursorLinePos >= maxRenderableLines + lineScrollOffsetY - 1)
        lineScrollOffsetY = cursorLinePos - maxRenderableLines + 1;
    else if (cursorLinePos < lineScrollOffsetY && lineScrollOffsetY > 0)
        lineScrollOffsetY = cursorLinePos;

    // RENDERING
    text = controller->getText();
    
    std::size_t numCharsRendered = 0;
    std::size_t numLinesToRender = std::min(maxRenderableLines, lineStartOffsets.size() - lineScrollOffsetY);

    for (std::size_t lineIndex = 0; lineIndex < numLinesToRender; ++lineIndex)
    {
        std::size_t scrolledLineIndex = lineIndex + lineScrollOffsetY;
        std::size_t lineStart = lineStartOffsets[scrolledLineIndex];
        std::size_t lineEnd;
        
        if (scrolledLineIndex + 1 < lineStartOffsets.size())
            lineEnd = lineStartOffsets[scrolledLineIndex + 1];
        else
            lineEnd = text.size();
        
        std::size_t len = lineEnd - lineStart;
        
        std::string lineBuffer = text.substr(lineStart, len);
        ImVec2 linePos = ImVec2(baseX, baseY + lineIndex * lineHeight);
        drawList->AddText(linePos, IM_COL32_WHITE, lineBuffer.c_str());
        numCharsRendered += lineBuffer.size();
    }

    // Cursor line/column
    auto it = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), cursorPos);
    std::size_t cursorLine = std::distance(lineStartOffsets.begin(), it) - 1;
    std::size_t cursorColumn = cursorPos - lineStartOffsets[cursorLine];

    float visualCursorX = baseX + cursorColumn * charWidth;
    float visualCursorY = baseY + (cursorLine - lineScrollOffsetY) * lineHeight;

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

    // Draw text selection highlight
    if (hasSelection())
    {
        ImU32 bg_color = ImGui::GetColorU32(ImGuiCol_TextSelectedBg, 0.6f);
        
        std::size_t selStart = getSelectionStart();
        std::size_t selEnd = getSelectionEnd();
        
        // Find line/column for start and end positions
        auto startIt = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), selStart);
        std::size_t startLine = std::distance(lineStartOffsets.begin(), startIt) - 1;
        std::size_t startColumn = selStart - lineStartOffsets[startLine];
        
        auto endIt = std::upper_bound(lineStartOffsets.begin(), lineStartOffsets.end(), selEnd);
        std::size_t endLine = std::distance(lineStartOffsets.begin(), endIt) - 1;
        std::size_t endColumn = selEnd - lineStartOffsets[endLine];
        
        if (startLine == endLine)
        {
            // Single line selection
            float startX = baseX + startColumn * charWidth;
            float endX = baseX + endColumn * charWidth;
            float y = baseY + (startLine - lineScrollOffsetY) * lineHeight;
            
            drawList->AddRectFilled(
                ImVec2(startX, y),
                ImVec2(endX, y + lineHeight),
                bg_color
            );
        }
        else
        {
            // Multi-line selection
            // First line: from start column to end of line
            float startX = baseX + startColumn * charWidth;
            float startY = baseY + (startLine - lineScrollOffsetY) * lineHeight;
            
            // Calculate end of first line
            std::size_t firstLineEnd = (startLine + 1 < lineStartOffsets.size()) ? 
                lineStartOffsets[startLine + 1] - lineStartOffsets[startLine] - 1 : 
                text.size() - lineStartOffsets[startLine];
            float firstLineEndX = baseX + firstLineEnd * charWidth;
            
            drawList->AddRectFilled(
                ImVec2(startX, startY),
                ImVec2(firstLineEndX, startY + lineHeight),
                bg_color
            );
            
            // Middle lines: full width
            for (size_t line = std::max(startLine + 1, lineScrollOffsetY); line < std::min(endLine, lineScrollOffsetY + numLinesToRender); ++line)
            {
                float y = baseY + (line - lineScrollOffsetY) * lineHeight;
                size_t lineLength = (line + 1 < lineStartOffsets.size()) ?
                    lineStartOffsets[line + 1] - lineStartOffsets[line] - 1 :
                    text.size() - lineStartOffsets[line];
                float lineEndX = baseX + lineLength * charWidth;
                
                drawList->AddRectFilled(
                    ImVec2(baseX, y),
                    ImVec2(lineEndX, y + lineHeight),
                    bg_color
                );
            }
            
            // Last line: from start of line to end column
            float endX = baseX + endColumn * charWidth;
            float endY = baseY + (endLine - lineScrollOffsetY) * lineHeight;
            
            drawList->AddRectFilled(
                ImVec2(baseX, endY),
                ImVec2(endX, endY + lineHeight),
                bg_color
            );
        }
    }

    // Draw status bar
    ImVec2 statusBarPos = ImVec2(contentAreaOrigin.x, contentAreaOrigin.y + textAreaSize.y);
    ImVec2 statusBarMax = ImVec2(contentAreaOrigin.x + contentRegionAvail.x, statusBarPos.y + statusBarHeight);
    
    drawList->AddRectFilled(statusBarPos, statusBarMax, IM_COL32(45, 45, 48, 255));
    
    drawList->AddLine(
        ImVec2(statusBarPos.x, statusBarPos.y),
        ImVec2(statusBarPos.x + contentRegionAvail.x, statusBarPos.y),
        IM_COL32(70, 70, 70, 255), 1.0f);
    
    const float padding = 5.0f;
    ImVec2 textPos(statusBarPos.x + padding, statusBarPos.y + 2.0f);
    
    std::string clientIdStr = controller ? controller->getClientId() : "Unknown";
    
    std::string statusText = clientIdStr + " | ";
    statusText += "Characters: " + std::to_string(text.size()) + " | ";
    statusText += "Line: " + std::to_string(cursorLine + 1) + ", ";
    statusText += "Column: " + std::to_string(cursorColumn + 1);
    
    drawList->AddText(textPos, IM_COL32(180, 180, 180, 255), statusText.c_str());

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
        
        if (hasSelection())
            deleteSelectedText(cursorPos);
        
        controller->handleTextInputEvent(TextInputEvent(TextInputEventType::INSERT, inputText, cursorPos, inputText.size()));
        controller->handleCursorInputEvent(CursorInputEvent(cursorPos + inputText.size()));
        onCursorMoved();
    }
}

bool Editor::hasSelection() const
{
    return selectionStartPos != selectionEndPos;
}

std::size_t Editor::getSelectionStart() const
{
    return std::min(selectionStartPos, selectionEndPos);
}

std::size_t Editor::getSelectionEnd() const
{
    return std::max(selectionStartPos, selectionEndPos);
}

void Editor::deleteSelectedText(std::size_t& cursorPos)
{
    size_t selStart = getSelectionStart();
    size_t selEnd = getSelectionEnd();
    
    controller->handleTextInputEvent(TextInputEvent(TextInputEventType::DELETE, "\0", selStart, selEnd - selStart));

    controller->handleCursorInputEvent(CursorInputEvent(selStart));
    cursorPos = selStart;
    
    selectionStartPos = 0;
    selectionEndPos = 0;
}

void Editor::onCursorMoved()
{
    cursorLastMovedTime = ImGui::GetTime();
    
    // Clear selection when cursor is moved via keyboard
    selectionStartPos = 0;
    selectionEndPos = 0;
}

std::size_t Editor::mousePosToCharPos(float baseX, float baseY, float charWidth, float lineHeight, std::size_t textLength)
{
    std::size_t xIndex = (ImGui::GetMousePos().x - baseX) / charWidth;
    std::size_t yIndex = (ImGui::GetMousePos().y - baseY) / lineHeight;
    
    // Add scroll offset to calculate the actual line in the document
    yIndex += lineScrollOffsetY;
    
    // Ensure we don't go beyond the available lines
    yIndex = std::min(yIndex, lineStartOffsets.size() - 1);

    std::size_t lineLength;
    if (yIndex + 1 < lineStartOffsets.size())
        lineLength = lineStartOffsets[yIndex + 1] - lineStartOffsets[yIndex] - 1;
    else
        lineLength = textLength - lineStartOffsets[yIndex];

    xIndex = std::min(xIndex, lineLength);
    
    return static_cast<std::size_t>(lineStartOffsets[yIndex] + xIndex);
}
