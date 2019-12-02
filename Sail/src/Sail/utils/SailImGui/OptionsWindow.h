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
	void updateMap();
	void setDisabled(bool b = true);

private:
	Application* m_app;
	SettingStorage* m_settings;
	LevelSystem* m_levelSystem;
	bool m_disabled = false;

	void drawCrosshair();
	void drawMap();
};