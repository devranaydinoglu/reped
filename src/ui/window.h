#pragma once

#include <memory>

#include "setup_window.h"

class Window
{
private:
    SetupWindow setupWindow;
    bool openSetupWindow;

public:
    Window();
    void render();

    void setOnAppModeSelectedCallback(SetupWindow::SetupCompletedCallback callback)
    {
        setupWindow.setOnSetupCompletedCallback(std::move(callback));
    }
};
