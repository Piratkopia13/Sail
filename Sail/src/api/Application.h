#pragma once

#include <windowsx.h>
#include "../utils/Timer.h"
#include "Win32Window.h"
#include "DXAPI.h"
#include "../resources/ResourceManager.h"
#include "Input.h"

class Application {

public:
	Application(int windowWidth, int windowHeight, const char* windowTitle, HINSTANCE hInstance);
	virtual ~Application();

	int startGameLoop();

	// Required methods
	virtual int run() = 0;
	virtual void processInput(float dt) = 0;
	virtual void update(float dt) = 0;
	virtual void render(float dt) = 0;

	// Optional methods
	// resize() is called when the window is resized
	virtual void resize(int width, int height) {};

	static Application* getInstance();
	DXAPI* const getAPI();
	Win32Window* const getWindow();
	ResourceManager& getResourceManager();
	const UINT getFPS() const;

	Input& getInput();

private:
	static Application* m_instance;
	Win32Window m_window;
	DXAPI m_dxAPI;
	ResourceManager m_resourceManager;

	Timer m_timer;
	UINT m_fps;

	Input m_input;

};