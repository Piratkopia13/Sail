#pragma once

#include "SailImGuiWindow.h"
#include "Sail/Application.h"
#include "Sail/entities/systems/Gameplay/LevelSystem/LevelSystem.h"

class AudioSystem;

class OptionsWindow : public SailImGuiWindow {

public:
	OptionsWindow(bool showWindow = true);
	~OptionsWindow();

	virtual void renderWindow() override;
	bool renderGameOptions();
	void updateMap();

private:
	Application* m_app = nullptr;
	SettingStorage* m_settings = nullptr;
	LevelSystem* m_levelSystem = nullptr;
	AudioSystem* m_audioSystem = nullptr;

	//void provide(AudioEngine* ae);

	void drawCrosshair();
	void drawMap();
};