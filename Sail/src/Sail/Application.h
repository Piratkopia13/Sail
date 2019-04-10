#pragma once
#include "api/Mesh.h"

// TODO: remove all api specific includes
#include "API/DX11/Win32Window.h"
//#include "API/DX11/DX11API.h"
#include "API/DX11/Input.h"

#include "api/GraphicsAPI.h"
#include "api/Window.h"

#include "utils/Timer.h"
#include "resources/ResourceManager.h"
#include "events/IEventDispatcher.h"

class Application : public IEventDispatcher {

public:
	enum API {
		DX11, DX12
	};

public:
	Application(int windowWidth, int windowHeight, const char* windowTitle, HINSTANCE hInstance, API api = DX11);
	virtual ~Application();

	int startGameLoop();

	// Required methods
	virtual int run() = 0;
	virtual void processInput(float dt) = 0;
	virtual void update(float dt) = 0;
	virtual void render(float dt) = 0;
	virtual void dispatchEvent(Event& event) override { }

	template<typename T>
	T* const getAPI() {
		return static_cast<T*>(m_api.get());
	}
	GraphicsAPI* const getAPI() {
		return m_api.get();
	}

	static Application* getInstance();
	Window* const getWindow();
	ResourceManager& getResourceManager();
	const UINT getFPS() const;

	Input& getInput();

private:
	static Application* m_instance;
	std::unique_ptr<Window> m_window;
	std::unique_ptr<GraphicsAPI> m_api;
	ResourceManager m_resourceManager;

	Timer m_timer;
	UINT m_fps;

	Input m_input;

};