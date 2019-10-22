#pragma once

#include "SailImGuiWindow.h"

class InGameMenuWindow : public SailImGuiWindow {

public:
	InGameMenuWindow(bool showWindow = true);
	~InGameMenuWindow();

	virtual void renderWindow() override;

	bool popGameState();

private:
	bool m_popGameState;
};