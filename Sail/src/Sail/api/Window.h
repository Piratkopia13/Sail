#pragma once

#include <string>
#include <windows.h>

class Window {
public:
	friend class Application;

	struct WindowProps {
		HINSTANCE hInstance;
		uint32_t windowWidth = 1280;
		uint32_t windowHeight = 720;
	};
public:
	static Window* Create(const WindowProps& props);
	Window(const WindowProps& props) : windowWidth(props.windowWidth), windowHeight(props.windowHeight), isWindowMinimized(false), isWindowFocused(false) {}
	virtual ~Window() {}

	virtual bool initialize() = 0;

	virtual inline void setWindowTitle(const std::string& title) = 0;
	virtual inline uint32_t getWindowWidth() const { return windowWidth; };
	virtual inline uint32_t getWindowHeight() const { return windowHeight; };
	virtual inline bool isMinimized() const { return isWindowMinimized; };
	virtual inline bool isFocused() const { return isWindowFocused; };


protected:
	uint32_t windowWidth;
	uint32_t windowHeight;
	bool isWindowMinimized;
	bool isWindowFocused;

private:
	// NOTE: this method is only used internally by the sail and should not be called by the user
	// Returns true if the window has been resized
	// after the last call of this method
	virtual bool hasBeenResized() = 0;

};