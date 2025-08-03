#pragma once

#include <memory>

#include "setup_window.h"
#include "editor.h"

class Window
{
private:
    SetupWindow setupWindow;
    bool openSetupWindow;

    Editor editor;
    bool openEditor;

public:
    Window();
    void render();

    void setOnSetupCompletedCallback(SetupWindow::SetupCompletedCallback callback)
    {
        setupWindow.setOnSetupCompletedCallback(std::move(callback));
    }

    void onSetupCompleted();
};
