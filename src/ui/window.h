#pragma once

#include "setup_window.h"
#include "editor.h"

class Controller;

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
    
    void setEditorController(Controller* controller)
    {
        editor.setController(controller);
    }

    void onSetupCompleted();
};
