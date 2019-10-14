#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <functional>
#include "Sail/events/Events.h"

class Input : public IEventListener {
	friend class Application;
public:
	template <typename T>
	static T* GetInstance() { return static_cast<T*>(m_Instance); }
	static Input* GetInstance() { return m_Instance; }

	inline static bool IsKeyPressed(int keycode) { return (m_HandleKeyInput) ? m_Instance->isKeyPressedImpl(keycode) : false; }
	inline static bool WasKeyJustPressed(int keycode) { return (m_HandleKeyInput) ? m_Instance->wasKeyJustPressedImpl(keycode) : false; }

	inline static bool IsMouseButtonPressed(int button) { return (m_HandleMouseInput) ? m_Instance->isMouseButtonPressedImpl(button) : false; }
	inline static bool WasMouseButtonJustPressed(int button) { return (m_HandleMouseInput) ? m_Instance->wasMouseButtonJustPressedImpl(button) : false; }

	inline static glm::ivec2 GetMousePosition() { return m_Instance->getMousePositionImpl(); }
	inline static glm::ivec2 GetMouseDelta() { return m_Instance->getMouseDeltaImpl(); }
	
	static void HideCursor(bool hide) { m_Instance->hideCursorImpl(hide); };
	static bool IsCursorHidden() { return m_Instance->isCursorHiddenImpl(); };

	inline static void SetMouseInput(bool takeInput) { m_HandleMouseInput = takeInput; }
	inline static void SetKeyInput(bool takeInput) { m_HandleKeyInput = takeInput; }

	virtual bool onEvent(Event& event) { return true; }

protected:
	virtual bool isKeyPressedImpl(int keycode) = 0;
	virtual bool wasKeyJustPressedImpl(int keycode) = 0;

	virtual bool isMouseButtonPressedImpl(int button) = 0;
	virtual bool wasMouseButtonJustPressedImpl(int button) = 0;

	virtual glm::ivec2 getMousePositionImpl() = 0;
	virtual glm::ivec2 getMouseDeltaImpl() = 0;

	virtual void hideCursorImpl(bool hide) = 0;
	virtual bool isCursorHiddenImpl() = 0;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;

private:
	static Input* m_Instance;
	static bool m_HandleMouseInput;
	static bool m_HandleKeyInput;

};