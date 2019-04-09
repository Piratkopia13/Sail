#pragma once

#include <string>

class Window {
public:
	Window(HINSTANCE hInstance, unsigned int windowWidth = 1280, unsigned int windowHeight = 720) : windowWidth(windowWidth), windowHeight(windowHeight) {}
	virtual ~Window() {}

	virtual bool initialize() = 0;

	virtual void setWindowTitle(const std::string& title) = 0;
	virtual unsigned int getWindowWidth() const { return windowWidth; };
	virtual unsigned int getWindowHeight() const { return windowHeight; };


protected:
	unsigned int windowWidth;
	unsigned int windowHeight;

private:

};