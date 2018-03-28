#include "Input.h"
#include "Application.h"

using namespace DirectX;

Input::Input()
	: m_frameDeltaAccumulationMouseDX(0)
	, m_frameDeltaAccumulationMouseDY(0)
	, m_mouseDXSinceLastFrame(0)
	, m_mouseDYSinceLastFrame(0)
	, m_mouseButtons{ false, false }
	, m_mouseButtonsPressedSinceLastFrame{ false, false }
{
	// Init dxtk keyboard, mouse and gamepad input
	m_keyboard = std::make_unique<Keyboard>();
	m_gamepad = std::make_unique<GamePad>();
}

Input::~Input() {
}

void Input::showCursor(bool show) {
	ShowCursor(show);
}
bool Input::isCursorHidden() {
	CURSORINFO pci = { 0 };
	pci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&pci);
	return pci.flags == 0;
}

LONG Input::getMouseDX() {
	return m_mouseDXSinceLastFrame;
}
LONG Input::getMouseDY() {
	return m_mouseDYSinceLastFrame;
}
bool Input::isMouseBtnPressed(MouseButton btn) {
	return m_mouseButtons[btn];
}
bool Input::wasJustPressed(MouseButton btn) {
	return m_mouseButtons[btn] && !m_mouseButtonsPressedSinceLastFrame[btn];
}

void Input::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	UINT dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL) {
		return;
	}

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

	RAWINPUT* raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEMOUSE) {

		// Accumulate deltas until the next frame
		m_frameDeltaAccumulationMouseDX += raw->data.mouse.lLastX;
		m_frameDeltaAccumulationMouseDY += raw->data.mouse.lLastY;

		// Update mouse button
		if ((RI_MOUSE_LEFT_BUTTON_DOWN & raw->data.mouse.usButtonFlags) == RI_MOUSE_LEFT_BUTTON_DOWN)
			m_mouseButtons[MouseButton::LEFT] = true;
		if ((RI_MOUSE_LEFT_BUTTON_UP & raw->data.mouse.usButtonFlags) == RI_MOUSE_LEFT_BUTTON_UP)
			m_mouseButtons[MouseButton::LEFT] = false;
		if ((RI_MOUSE_RIGHT_BUTTON_DOWN & raw->data.mouse.usButtonFlags) == RI_MOUSE_RIGHT_BUTTON_DOWN)
			m_mouseButtons[MouseButton::RIGHT] = true;
		if ((RI_MOUSE_RIGHT_BUTTON_UP & raw->data.mouse.usButtonFlags) == RI_MOUSE_RIGHT_BUTTON_UP)
			m_mouseButtons[MouseButton::RIGHT] = false;

	}

	delete[] lpb;
}

const DirectX::Keyboard::State& Input::getKeyboardState() const {
	return m_keyboardState;
}

const DirectX::GamePad::State& Input::getGamePadState(int padIndex) const {
	if (padIndex < 0 || padIndex > 3)
		Logger::Error("Tried to get GamePadState of invalid pad index: " + padIndex);

	return m_gamepadState[padIndex];
}

const DirectX::Keyboard::KeyboardStateTracker& Input::getKbStateTracker() const {
	return m_kbTracker;
}

const DirectX::GamePad::ButtonStateTracker& Input::getGpStateTracker(int padIndex) const {
	if (padIndex < 0 || padIndex > 3)
		Logger::Error("Tried to get ButtonStateTracker of invalid pad index: " + padIndex);

	return m_gpTracker[padIndex];
}

DirectX::GamePad& Input::getGamePad() const {
	return *m_gamepad.get();
}

void Input::processAllGamepads(std::function<void(const DirectX::GamePad::State&, const DirectX::GamePad::ButtonStateTracker&)> processFunc) {
	for (int i = 0; i < 4; i++) {
		if (m_gamepadState[i].IsConnected())
			processFunc(m_gamepadState[i], m_gpTracker[i]);
	}
}

void Input::updateStates() {
	// Update input states for keyboard and controllers
	for (int i = 0; i < 4; i++)
		m_gamepadState[i] = GamePad::Get().GetState(i, GamePad::DEAD_ZONE_CIRCULAR);
	m_keyboardState = Keyboard::Get().GetState();

	// Update trackers
	for (int i = 0; i < 4; i++)
		m_gpTracker[i].Update(m_gamepadState[i]);
	m_kbTracker.Update(m_keyboardState);
}


void Input::registerRawDevices(HWND hwnd) {
	// Register mouse for raw input
	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_INPUTSINK;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = hwnd;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		OutputDebugString(L"Could not register mouse for raw input");
	}
}

void Input::newFrame() {
	// Set cursor position to center of window if the cursor is hidden
	if (isCursorHidden()) {
		POINT p;
		p.x = Application::getInstance()->getWindow()->getWindowWidth() / 2;
		p.y = Application::getInstance()->getWindow()->getWindowHeight() / 2;
		ClientToScreen(*Application::getInstance()->getWindow()->getHwnd(), &p);
		SetCursorPos(p.x, p.y);
	}

	m_mouseDXSinceLastFrame = m_frameDeltaAccumulationMouseDX;
	m_mouseDYSinceLastFrame = m_frameDeltaAccumulationMouseDY;
	m_frameDeltaAccumulationMouseDX = 0;
	m_frameDeltaAccumulationMouseDY = 0;
}
void Input::endOfFrame() {
	m_mouseButtonsPressedSinceLastFrame[0] = m_mouseButtons[0];
	m_mouseButtonsPressedSinceLastFrame[1] = m_mouseButtons[1];
}