#pragma once

#include "SailImGuiWindow.h"
#include "Sail/Application.h"
#include "Sail/entities/systems/Gameplay/LevelSystem/LevelSystem.h"

class OptionsWindow : public SailImGuiWindow {

public:
	OptionsWindow(bool showWindow = true);
	~OptionsWindow();

	virtual void renderWindow() override;
	bool renderGameOptions();

private:
	Application* m_app;
	SettingStorage* m_settings;
	LevelSystem* m_levelSystem;

	void drawCrosshair();
	void drawMap();
};