#pragma once

#include "SailImGuiWindow.h"

class InGameMenuWindow : public SailImGuiWindow {

public:
	InGameMenuWindow(bool showWindow = true);
	~InGameMenuWindow();

	virtual void renderWindow() override;

	bool popGameState();
	bool exitInGameMenu();
	bool showOptions();

private:
	bool m_openedThisFrame;

	bool m_popGameState;
	bool m_exitInGameMenu;
	bool m_showOptions;
};