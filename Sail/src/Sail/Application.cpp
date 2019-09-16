#include "pch.h"
#include "Application.h"
#include "events/WindowResizeEvent.h"
#include "KeyCodes.h"
#include "graphics/geometry/Transform.h"



Application* Application::m_instance = nullptr;

Application::Application(int windowWidth, int windowHeight, const char* windowTitle, HINSTANCE hInstance, API api) {

	// Set up instance if not set
	if (m_instance) {
		Logger::Error("Only one application can exist!");
		return;
	}
	m_instance = this;

	// Set up thread pool with two times as many threads as logical cores, or four threads if the CPU only has one core;
	// Note: this value might need future optimization
	unsigned int poolSize = std::max<unsigned int>(4, (2 * std::thread::hardware_concurrency()));
	m_threadPool = std::unique_ptr<ctpl::thread_pool>(new ctpl::thread_pool(poolSize));

	// Set up window
	Window::WindowProps windowProps;
	windowProps.hInstance = hInstance;
	windowProps.windowWidth = windowWidth;
	windowProps.windowHeight = windowHeight;
	m_window = std::unique_ptr<Window>(Window::Create(windowProps));
	m_window->setWindowTitle(windowTitle);

	// Set up api
	m_api = std::unique_ptr<GraphicsAPI>(GraphicsAPI::Create());
	// Set up imgui handler
	m_imguiHandler = std::unique_ptr<ImGuiHandler>(ImGuiHandler::Create());

	// Initialize the window
	if (!m_window->initialize()) {
		OutputDebugString(L"\nFailed to initialize Win32Window\n");
		Logger::Error("Failed to initialize Win32Window!");
		return;
	}

	// Initialize the graphics API
	if (!m_api->init(m_window.get())) {
		OutputDebugString(L"\nFailed to initialize the graphics API\n");
		Logger::Error("Failed to initialize the graphics API!");
		return;
	}

	// Initialize imgui
	m_imguiHandler->init();

	// Register devices to use raw input from hardware
	//m_input.registerRawDevices(*m_window.getHwnd());

	// Load the missing texture texture
	m_resourceManager.loadTexture("missing.tga");

}

Application::~Application() {
	delete Input::GetInstance();
}


// CAUTION: HERE BE DRAGONS!
// Update and render synchronization is not guaranteed to work without data races when the framerate is significantly lower
// than the update rate
int Application::startGameLoop() {
	MSG msg = {0};
	m_fps = 0;

	double currentTime = m_startTime;
	double newTime = 0.0;
	double delta = 0.0;
	double accumulator = 0.0;

	double secCounter = 0.0;
	double elapsedTime = 0.0;
	UINT frameCounter = 0;


	// Start delta timer
	m_timer.startTimer();
	
	// Set the start time so that the update and render loops use the same epoch with the getTimeSince(time) function in Timer.h
	m_startTime = m_timer.getTime();

	// Render loop, each iteration of it results in one rendered frame
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// Handle window resizing
			if (m_window->hasBeenResized()) {
				UINT newWidth = m_window->getWindowWidth();
				UINT newHeight = m_window->getWindowHeight();
				bool isMinimized = m_window->isMinimized();
				// Send resize event
				dispatchEvent(WindowResizeEvent(newWidth, newHeight, isMinimized));
			}

			newTime = m_timer.getTimeSince(m_startTime);
			delta = newTime - currentTime;
			currentTime = newTime;
			//delta = std::min(delta, 0.25); // might be needed for correct interpolation

			// Get delta time from last frame
			//float delta = static_cast<float>(m_timer.getFrameTime());
			//delta = std::min(delta, 0.04f);
			//delta = std::min(delta, 0.04); // needed?

			// Update fps counter
			//secCounter += delta;
			secCounter += delta;
			frameCounter++;

			if (secCounter >= 1) {
				m_fps = frameCounter;
				frameCounter = 0;
				secCounter = 0.f;
			}

			// Update mouse deltas
			Input::GetInstance()->beginFrame();

			// Quit on alt-f4
			if (Input::IsKeyPressed(SAIL_KEY_MENU) && Input::IsKeyPressed(SAIL_KEY_F4))
				PostQuitMessage(0);

#ifdef _DEBUG
			/*if (m_input.getKeyboardState().Escape)
			PostQuitMessage(0);*/
			//if(delta > 0.0166)
			//	Logger::Warning(std::to_string(elapsedTime) + " delta over 0.0166: " + std::to_string(delta));
#endif

			// Process state specific input
			// NOTE: player movement is updated in update() and mouse movement is updated in render()
			processInput(delta);

			accumulator += delta;

			// Queue multiple updates if the update has fallen behind to make sure that it catches back up to the current time.
			int CPU_updatesThisLoop = 0;
			while (accumulator >= TIMESTEP) {
				accumulator -= TIMESTEP;
				CPU_updatesThisLoop++;
			}

			// Run update(s) in a separate thread
			m_threadPool->push([this, CPU_updatesThisLoop](int id) {
				int updatesRemaining = CPU_updatesThisLoop;
				while (updatesRemaining > 0) {
					Transform::incrementCurrentUpdateIndex();
					updatesRemaining--;
					update(TIMESTEP);
				}
				});

			// Render
			Transform::updateCurrentRenderIndex();
			render(delta);


			//// FOR DEBUGGING
			//UINT a = Transform::getUpdateIndex();
			//UINT b = Transform::getRenderIndex();

			// Reset just pressed keys
			Input::GetInstance()->endFrame();
		}
	}

	return (int)msg.wParam;

}

std::string Application::getPlatformName() {
	return std::string(SAIL_PLATFORM);
}

Application* Application::getInstance() {
	if (!m_instance)
		Logger::Error("Application instance not set, you need to initialize the class which inherits from Application before calling getInstance().");
	return m_instance;
}

void Application::dispatchEvent(Event& event) {
	m_api->onEvent(event);
	Input::GetInstance()->onEvent(event);
}

GraphicsAPI* const Application::getAPI() {
	return m_api.get();
}
Window* const Application::getWindow() {
	return m_window.get();
}
ImGuiHandler* const Application::getImGuiHandler() {
	return m_imguiHandler.get();
}
ResourceManager& Application::getResourceManager() {
	return m_resourceManager;
}
const UINT Application::getFPS() const {
	return m_fps;
}
