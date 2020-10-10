#pragma once
#include "api/Mesh.h"

#include "api/Input.h"
#include "api/GraphicsAPI.h"
#include "api/Window.h"
#include "api/gui/ImGuiHandler.h"

#include "utils/Timer.h"
#include "resources/ResourceManager.h"
#include "Settings.h"

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

	template<typename T>
	T* const getAPI() { return static_cast<T*>(m_api.get()); }
	GraphicsAPI* const getAPI();

	template<typename T>
	T* const getWindow() { return static_cast<T*>(m_window.get()); }
	Window* const getWindow();

	// Schedule a function to execute at the beginning of the next frame
	// Should be used when modifying resources that may be in flight
	void scheduleForNextFrame(std::function<void()> func);
	void pauseRendering(bool pause);

	static std::string getPlatformName();
	static Application* getInstance();
	ImGuiHandler* const getImGuiHandler();
	ResourceManager& getResourceManager();
	Settings& getSettings();
	const UINT getFPS() const;

private:
	static Application* m_instance;
	std::unique_ptr<Window> m_window;
	std::unique_ptr<GraphicsAPI> m_api;
	std::unique_ptr<ImGuiHandler> m_imguiHandler;
	ResourceManager m_resourceManager;
	Settings m_settings;
	bool m_pauseRendering;

	Timer m_timer;
	UINT m_fps;

	std::vector<std::function<void()>> m_scheduledFuncsForNextFrame;
};