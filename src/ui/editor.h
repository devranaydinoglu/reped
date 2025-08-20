#pragma once

#include <cstddef>
#include <vector>

struct ImGuiInputTextCallbackData;
class Controller;

class Editor
{
private:
    Controller* controller;

    // Character offset of the start of each line
    std::vector<std::size_t> lineStartOffsets;

    float cursorLastMovedTime;

public:
    Editor();
    void showEditor(bool* open);
    void setController(Controller* controller) { this->controller = controller; }
    void handleTextInput(const char* text);

private:
    std::size_t mousePosToCharPos(float baseX, float baseY, float charWidth, float lineHeight, std::size_t textLength);
};
