#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>
#include <functional>
#include <Keyboard.h>
#include <Mouse.h>
#include <GamePad.h>

class Input {
	friend class Application;
public:
	enum MouseButton {
		LEFT,
		RIGHT
	};

	Input();
	~Input();

	void showCursor(bool show);
	bool isCursorHidden();

	LONG getMouseDX();
	LONG getMouseDY();
	bool isMouseBtnPressed(MouseButton btn);
	bool wasJustPressed(MouseButton btn);

	void processMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	const DirectX::Keyboard::State& getKeyboardState() const;
	const DirectX::GamePad::State& getGamePadState(int padIndex) const;
	const DirectX::Keyboard::KeyboardStateTracker& getKbStateTracker() const;
	const DirectX::GamePad::ButtonStateTracker& getGpStateTracker(int padIndex) const;
	DirectX::GamePad& getGamePad() const;
	void processAllGamepads(std::function<void(const DirectX::GamePad::State&, const DirectX::GamePad::ButtonStateTracker&)> processFunc);

	void updateStates();

private:
	void registerRawDevices(HWND hwnd);
	void newFrame();
	void endOfFrame();

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	DirectX::Keyboard::State m_keyboardState;
	DirectX::Keyboard::KeyboardStateTracker m_kbTracker;

	std::unique_ptr<DirectX::GamePad> m_gamepad;
	DirectX::GamePad::State m_gamepadState[4];
	DirectX::GamePad::ButtonStateTracker m_gpTracker[4];

	bool m_mouseButtons[2];
	bool m_mouseButtonsPressedSinceLastFrame[2];

	LONG m_frameDeltaAccumulationMouseDX;
	LONG m_frameDeltaAccumulationMouseDY;
	LONG m_mouseDXSinceLastFrame;
	LONG m_mouseDYSinceLastFrame;

};