#pragma once

#include "SailImGuiWindow.h"
#include <map>

class WaitingForPlayersWindow : public SailImGuiWindow {

public:
	WaitingForPlayersWindow(bool showWindow = true);
	~WaitingForPlayersWindow();

	virtual void renderWindow() override;

private:

};