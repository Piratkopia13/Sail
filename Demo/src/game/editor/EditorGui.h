#pragma once

#include <functional>
#include <windows.h>
#include "Sail/api/gui/SailGuiWindow.h"

#define FUNC(a) std::function<a>

class Entity;

class EditorGui : public SailGuiWindow {
public:
    enum CallbackType {
        CHANGE_STATE,
        ENVIRONMENT_CHANGED,
    };

    EditorGui();
    void render(float dt, FUNC(void(CallbackType type, const std::string&)) callback);

private:
    void setupDockspace(float menuBarHeight);
    // Returns height of the menu bar
	float setupMenuBar();

private:
    FUNC(void(CallbackType type, const std::string&)) m_callback;

    std::string m_modelName;
};