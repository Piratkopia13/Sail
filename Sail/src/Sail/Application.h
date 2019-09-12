#pragma once
#include "api/Mesh.h"

#include "api/Input.h"
#include "api/GraphicsAPI.h"
#include "api/Window.h"
#include "api/ImGuiHandler.h"

#include "utils/Timer.h"
#include "resources/ResourceManager.h"
#include "events/IEventDispatcher.h"

//#include "graphics/FrameSynchronizer.h"



// used in template functions
#include <future>

// Forward declarations
namespace ctpl {
	class thread_pool;
}


// TODO: Move elsewhere
//const unsigned int SNAPSHOT_BUFFER_SIZE = 4;
const double TICKRATE = 100.0;
const double TIMESTEP = 1.0 / TICKRATE;



class Application : public IEventDispatcher {

public:
	enum API {
		DX11, DX12
	};

public:
	Application(int windowWidth, int windowHeight, const char* windowTitle, HINSTANCE hInstance, API api = DX11);
	virtual ~Application();

	int startGameLoop();

	void startUpdateAndRenderLoops();
	void startUpdateLoop();
	void startRenderLoop();

	// Required methods
	virtual int run() = 0;
	virtual void processInput(float dt) = 0;
	virtual void update(float dt) = 0;
	virtual void render(float dt) = 0;
	virtual void dispatchEvent(Event& event) override;

	template<typename T>
	T* const getAPI() { return static_cast<T*>(m_api.get()); }
	GraphicsAPI* const getAPI();

	template<typename T>
	T* const getWindow() { return static_cast<T*>(m_window.get()); }
	Window* const getWindow();

	// Pass-through functions for pushing jobs (functions, lambdas, etc.) to the thread pool.
	// The first parameter of all jobs must be int id which becomes the id of the running thread.
	// Returns std::future containing the return type of the job.
	//
	// EXAMPLE USAGE:
	// To push a member function from another member function use a lambda which captures this and other relevant arguments:
	//     float f = 10.0f;
	//     pushJobToThreadPool([this,f](int id) { return this->memberFunction(id, f); });
	//
	// To push a member function in another object use a lambda which captures that object and other relevant arguments:
	//     Object obj;
	//     pushJobToThreadPool([&obj, f](int id) { return obj.memberFunction(id, f); });
	//
	// See https://github.com/vit-vit/CTPL for other examples with standalone functions, lambdas, etc.
	template<typename F, typename... Rest>
	auto pushJobToThreadPool(F&& f, Rest&& ... rest)->std::future<decltype(f(0, rest...))> {
		return m_threadPool->push(f, std::forward<Rest>(rest)...);
	}
	template<typename F>
	auto pushJobToThreadPool(F&& f)->std::future<decltype(f(0))> {
		return m_threadPool->push(f);
	}

	static std::string getPlatformName();
	static Application* getInstance();
	ImGuiHandler* const getImGuiHandler();
	ResourceManager& getResourceManager();
	const UINT getFPS() const;


	// To be done at the end of each CPU update and nowhere else
	void incrementFrameIndex();
	unsigned int getFrameIndex() const;
	unsigned int getSnapshotBufferIndex() const;
private:
	static Application* m_instance;
	std::unique_ptr<Window> m_window;
	std::unique_ptr<GraphicsAPI> m_api;
	std::unique_ptr<ImGuiHandler> m_imguiHandler;
	ResourceManager m_resourceManager;

	std::unique_ptr<ctpl::thread_pool> m_threadPool;

	//std::unique_ptr<FrameSynchronizer> m_frameSynchronizer;

	// first frame is 0 and it continues from there, integer overflow isn't a problem unless
	// you leave the game running for like a year or two.
	std::atomic_uint m_frameInd = 0;

	// the index in the snapshot buffer that is used in the update loop on the CPU.
	// [0, SNAPSHOT_BUFFER_SIZE-1]
	// If CPU update is working on index 3 then prepare render will safely interpolate between
	// index 1 and 2 without any data races
	std::atomic_uint m_snapshotBufInd = 0;

	std::atomic_bool m_isRunning = true;

	double m_startTime;

	Timer m_updateTimer;
	Timer m_renderTimer;

	UINT m_fps;

};