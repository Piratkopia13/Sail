#pragma once

class SailImGuiWindow {
public:
	SailImGuiWindow();
	SailImGuiWindow(bool showWindow);
	virtual ~SailImGuiWindow();

	bool isWindowOpen();
	virtual void toggleWindow();
	virtual void showWindow(bool show);
	virtual void renderWindow() = 0;

private:
	bool m_isWindowOpen;
};