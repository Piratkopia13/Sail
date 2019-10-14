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

	static float EaseInOut(float time, float startValue, float endValue, float duration);
	static float EaseIn(float time, float startValue, float endValue, float duration);
	static float EaseOut(float time, float startValue, float endValue, float duration);

private:
	bool m_isWindowOpen;
};