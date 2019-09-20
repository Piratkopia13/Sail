#pragma once

#include "Sail/api/Input.h"
#include "../../Sail/MouseButtonCodes.h"
#include "../../Sail/KeyCodes.h"


class Win32Input : public Input {
public:
	Win32Input();
	~Win32Input();

	void registerRawDevices(HWND hwnd);
	void processMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual bool onEvent(Event& event) override;

private:
	bool m_cursorHidden;
	int m_mouseDX;
	int m_mouseDY;
	glm::ivec2 m_mouseDelta;
	glm::ivec2 m_mousePos;
	bool m_mouseButtons[SAIL_NUM_MOUSE_BUTTONS];
	bool m_keys[SAIL_NUM_KEYS];
	// frame_ contains keys and buttons pressed this frame
	bool m_frameMouseButtons[SAIL_NUM_MOUSE_BUTTONS];
	bool m_frameKeys[SAIL_NUM_KEYS];
	bool m_stopInput; // Used to stop input when window is not in focus

protected:
	virtual bool isKeyPressedImpl(int keycode) override;
	virtual bool wasKeyJustPressedImpl(int keycode) override;

	virtual bool isMouseButtonPressedImpl(int button) override;
	virtual bool wasMouseButtonJustPressedImpl(int button) override;

	virtual glm::ivec2 getMousePositionImpl() override;
	virtual glm::ivec2 getMouseDeltaImpl() override;

	virtual void hideCursorImpl(bool hide) override;
	virtual bool isCursorHiddenImpl() override;

	virtual void beginFrame() override;
	virtual void endFrame() override;

};