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

	static float EaseInOut(float time, float startValue, float endValue, float duration) {
		time /= duration / 2;
		if (time < 1) return endValue / 2 * time * time + startValue;
		time--;
		return -endValue / 2 * (time * (time - 2) - 1) + startValue;
	}

private:
	bool m_isWindowOpen;
};