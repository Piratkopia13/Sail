#include "Win32Window.h"
#include "Application.h"

using namespace DirectX;

namespace {
	// Used to forward messages to user defined proc function
	Win32Window* g_pApp = nullptr;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (g_pApp) return g_pApp->MsgProc(hwnd, msg, wParam, lParam);
	else return DefWindowProc(hwnd, msg, wParam, lParam);
}

Win32Window::Win32Window(HINSTANCE hInstance, int windowWidth, int windowHeight, const char* windowTitle)
: m_hWnd(NULL)
, m_hInstance(hInstance)
, m_windowWidth(windowWidth)
, m_windowHeight(windowHeight)
, m_windowTitle(windowTitle)
, m_windowStyle(WS_OVERLAPPEDWINDOW) // Default window style
, m_resized(false)
{
	g_pApp = this;
}

Win32Window::~Win32Window() {
}


bool Win32Window::initialize() {

	// WNDCLASSEX
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_hInstance;
	wcex.lpfnWndProc = MainWndProc;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Win32Window";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) {
		OutputDebugString(L"\nFailed to create window class\n");
		throw std::exception();
		return false;
	}

	// Get the correct width and height (windows includes title bar in size)
	RECT r = { 0L, 0L, static_cast<LONG>(m_windowWidth), static_cast<LONG>(m_windowHeight) };
	AdjustWindowRect(&r, m_windowStyle, FALSE);
	UINT width = r.right - r.left;
	UINT height = r.bottom - r.top;

	UINT x = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
	UINT y = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;

	std::wstring stemp = std::wstring(m_windowTitle.begin(), m_windowTitle.end());
	LPCWSTR title = stemp.c_str();

	m_hWnd = CreateWindow(L"Win32Window", title, m_windowStyle, x, y, width, height, NULL, NULL, m_hInstance, NULL);

	if (!m_hWnd) {
		OutputDebugString(L"\nFailed to create window\n");
		throw std::exception();
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);

	return true;

}

LRESULT Win32Window::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;

	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(msg, wParam, lParam);
		break;

	case WM_INPUT:
		Application::getInstance()->getInput().processMessage(msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(msg, wParam, lParam);
		break;

	case WM_SIZE:
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			if (width != m_windowWidth || height != m_windowHeight) {
				m_windowWidth = width;
				m_windowHeight = height;
				m_resized = true;
			}
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;

}

bool Win32Window::hasBeenResized() {
	bool ret = m_resized;
	m_resized = false;
	return ret;
}

const HWND* Win32Window::getHwnd() const {
	return &m_hWnd;
}

UINT Win32Window::getWindowWidth() const {
	return m_windowWidth;
}

UINT Win32Window::getWindowHeight() const {
	return m_windowHeight;
}
