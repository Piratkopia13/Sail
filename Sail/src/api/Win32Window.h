#pragma once

#include <Windows.h>
#include <string>

class Win32Window {

public:
	Win32Window(HINSTANCE hInstance, int windowWidth, int windowHeight, const char* windowTitle);
	~Win32Window();

	bool initialize();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	// NOTE: this method is only used internally by the sail and should not be called by the user
	// Returns true if the window has been resized
	// after the last call of this method
	bool hasBeenResized();

	const HWND* getHwnd() const;
	UINT getWindowWidth() const;
	UINT getWindowHeight() const;

private:

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	UINT m_windowWidth;
	UINT m_windowHeight;
	std::string m_windowTitle;
	DWORD m_windowStyle;
	bool m_resized;

};