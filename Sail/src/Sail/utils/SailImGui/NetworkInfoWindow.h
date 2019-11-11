#pragma once

#include "SailImGuiWindow.h"

class NetworkInfoWindow : public SailImGuiWindow {
public:
	NetworkInfoWindow(bool showWindow = true);
	~NetworkInfoWindow();

	virtual void renderWindow() override;

private:

};