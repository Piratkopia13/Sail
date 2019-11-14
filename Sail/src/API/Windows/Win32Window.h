#pragma once

#include "Sail/api/Window.h"

#include <Windows.h>
#include <string>

// Forward declaration
struct IDXGISwapChain;

class Win32Window : public Window {

public:
	Win32Window(const WindowProps& props);
	~Win32Window();

	virtual bool initialize() override;
	virtual bool hasBeenResized() override;
	virtual void setWindowTitle(const std::string& title) override;

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	const HWND* getHwnd() const;
	DWORD getWindowStyle() const;

private:

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	std::string m_windowTitle;
	DWORD m_windowStyle;
	bool m_resized;

};