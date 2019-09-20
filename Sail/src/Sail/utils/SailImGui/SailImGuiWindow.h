#pragma once

class SailImGuiWindow {
public:
	SailImGuiWindow();
	SailImGuiWindow(const bool state);
	~SailImGuiWindow();

	const bool windowOpen();
	void toggle();
	void windowState(const bool state);

private:
	bool m_windowState;
};