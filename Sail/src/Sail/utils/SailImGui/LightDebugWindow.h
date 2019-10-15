#pragma once

#include "SailImGuiWindow.h"
class LightSetup;

class LightDebugWindow : public SailImGuiWindow {

public:
	LightDebugWindow(bool showWindow = true);
	~LightDebugWindow();

	void setLightSetup(LightSetup* lights);
	bool isManualOverrideOn();
	void setManualOverride(bool enable);

	virtual void renderWindow() override;

private:
	bool m_manualOverride;
	LightSetup* m_lightSetup;
};