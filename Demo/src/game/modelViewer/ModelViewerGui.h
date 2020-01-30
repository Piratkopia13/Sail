#pragma once

#include <functional>
#include <windows.h>

#define FUNC(a) std::function<a>

class ModelViewerGui {
public:
    ModelViewerGui();
    void render(float dt, FUNC(void()) funcSwitchState, FUNC(void(const std::string&)) callbackNewModel);

private:
    void setupDockspace(float menuBarHeight);
    // Returns height of the menu bar
	float setupMenuBar();

    // Utils

	// Returns an empty string if dialog is canceled
    // TODO: make method cross platform
    std::string openFilename(LPCWSTR filter = L"All Files (*.*)\0*.*\0", HWND owner = NULL);

private:
    FUNC(void()) m_funcSwitchState;
    FUNC(void(const std::string&)) m_callbackNewModel;

    std::string m_modelName;
};