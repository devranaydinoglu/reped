#pragma once

#include <functional>

enum class AppMode;

class SetupWindow
{
public:
    using SetupCompletedCallback = std::function<void(AppMode appMode, const uint16_t port, const std::string& serverAddress, const std::string& clientId, const std::string& filePathName)>;

private:
    char inputClientId[20];
    char inputPort[6];
    char inputAddress[18];
    std::string filePathName;

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