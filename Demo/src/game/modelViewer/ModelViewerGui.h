#pragma once

#include <functional>
#include <windows.h>
#include "Sail/api/gui/SailGuiWindow.h"

#define FUNC(a) std::function<a>

class Entity;

class ModelViewerGui : public SailGuiWindow {
public:
    enum CallbackType {
        CHANGE_STATE,
        MODEL_CHANGED,
        ENVIRONMENT_CHANGED,
    };

    ModelViewerGui();
    void render(float dt, FUNC(void(CallbackType type, const std::string&)) callback, Entity* entity);

    // SailGuiWindow overrides
	virtual void addProperty(const char* label, std::function<void()> prop) override;
	virtual void setOption(const std::string& optionName, bool value) override;
    virtual void newSection(const std::string& title) override;

private:
    void setupDockspace(float menuBarHeight);
    // Returns height of the menu bar
	float setupMenuBar();

	void enableColumns(float labelWidth = 75.f);
	void disableColumns();

private:
    FUNC(void(CallbackType type, const std::string&)) m_callback;

    std::string m_modelName;

	float m_propWidth;
	int m_propID;
    bool m_setNextPropWidth;
};