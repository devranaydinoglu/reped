#pragma once

#include <functional>

enum class AppMode;

class SetupWindow
{
public:
    using SetupCompletedCallback = std::function<void(AppMode appMode, const uint16_t port, const std::string& serverAddress)>;

private:
    char inputId[20];
    char inputPort[6];
    char inputAddress[18];

public:
    SetupWindow();
    void showSetupWindow(bool* open);

    void setOnSetupCompletedCallback(SetupCompletedCallback callback)
    {
        setupCompletedCallback = std::move(callback);
    }

private:
    SetupCompletedCallback setupCompletedCallback;
};