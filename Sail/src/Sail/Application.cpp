#include "pch.h"
#include "Application.h"
#include "events/WindowResizeEvent.h"
#include "KeyCodes.h"




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

int Application::startGameLoop() {

	// Start delta timers
	m_updateTimer.startTimer();
	m_renderTimer.startTimer();
	

	// Set the start time so that the update and render loops use the same time with the getTimeSince(time) function in Timer.h
	m_startTime = m_updateTimer.getTime();

	MSG msg = {0};

	m_fps = 0;

	float updateTimer = 0.f;
	float timeBetweenUpdates = 1.f / 60.f;

	double currentTime = m_startTime;
	double accumulator = 0.0;


	float secCounter = 0.f;
	float elapsedTime = 0.f;
	UINT frameCounter = 0;

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

			// Get delta time from last frame
			float delta = static_cast<float>(m_updateTimer.getFrameTime());
			delta = std::min(delta, 0.04f);

			// Update fps counter
			secCounter += delta;
			frameCounter++;

			if (secCounter >= 1) {
				m_fps = frameCounter;
				frameCounter = 0;
				secCounter = 0.f;
			}

			// Update input states
			//m_input.updateStates();

			// Update mouse deltas
			Input::GetInstance()->beginFrame();

			// Quit on alt-f4
			if (Input::IsKeyPressed(SAIL_KEY_MENU) && Input::IsKeyPressed(SAIL_KEY_F4))
				PostQuitMessage(0);

			processInput(delta);

			// Update
#ifdef _DEBUG
			/*if (m_input.getKeyboardState().Escape)
			PostQuitMessage(0);*/


			//if(delta > 0.0166)
			//	Logger::Warning(std::to_string(elapsedTime) + " delta over 0.0166: " + std::to_string(delta));
#endif
			double newTime = m_updateTimer.getTimeSince(m_startTime);
			double frameTime = newTime - currentTime;
			if (frameTime > 0.25) {
				frameTime = 0.25;
			}
			currentTime = newTime;

			accumulator += frameTime;

			if (accumulator >= TIMESTEP) {
				accumulator -= TIMESTEP;
				m_threadPool->push([this](int id) {
						incrementFrameIndex();
						update(TIMESTEP); 
					});


			/*	update(TIMESTEP);
				incrementFrameIndex();*/
				
				//updateTimer -= timeBetweenUpdates;
			}

			// Render
			render(delta, getSnapshotBufferIndex());

			// Reset just pressed keys
			Input::GetInstance()->endFrame();
		}

	}

	m_isRunning = false;

	return (int)msg.wParam;

}

void Application::startUpdateAndRenderLoops() {
	//m_threadPool->push([this](int id) {startUpdateLoop(); });
	//m_threadPool->push([this](int id) {startRenderLoop(); });
}


// TODO: rewrite in a simpler way since render is now completely separate
void Application::startUpdateLoop() {
	////double t = 0.0;
	////double dt = TIMESTEP;

	//double currentTime = m_startTime;
	//double accumulator = 0.0;

	//while (m_isRunning.load()) {
	//	double newTime = m_updateTimer.getTimeSince(m_startTime);
	//	double frameTime = newTime - currentTime;
	//	if (frameTime > 0.25) {
	//		frameTime = 0.25;
	//	}
	//	currentTime = newTime;

	//	accumulator += frameTime;

	//	while (accumulator >= TIMESTEP) {
	//		// Update mouse deltas
	//		Input::GetInstance()->beginFrame();

	//		// Quit on alt-f4
	//		if (Input::IsKeyPressed(SAIL_KEY_MENU) && Input::IsKeyPressed(SAIL_KEY_F4))
	//			PostQuitMessage(0);


	//		// TODO: separate camera update
	//		processInput(TIMESTEP);


	//		incrementFrameIndex(); // ? do here or in gamestate update?
	//		update(TIMESTEP);
	//		//t += dt;
	//		accumulator -= TIMESTEP;

	//		// Reset just pressed keys
	//		Input::GetInstance()->endFrame();
	//	}

	//	// TODO: interpolate between game states in render
	//}
}

void Application::startRenderLoop() {
	//float secCounter = 0.f;
	////float elapsedTime = 0.f;
	//UINT frameCounter = 0;

	//while (m_isRunning.load()) {
	//	// Get delta time from last frame
	//	float delta = static_cast<float>(m_updateTimer.getFrameTime());
	//	delta = std::min(delta, 0.04f);

	//	// Update fps counter
	//	secCounter += delta;
	//	frameCounter++;

	//	if (secCounter >= 1) {
	//		m_fps = frameCounter;
	//		frameCounter = 0;
	//		secCounter = 0.f;
	//	}


	//	double alpha = std::fmod(m_renderTimer.getTimeSince(m_startTime), TIMESTEP);
	//	render(alpha);
	//}
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

// To be done at the end of each CPU update and nowhere else
void Application::incrementFrameIndex() {
	m_snapshotBufInd = ((++m_frameInd) % SNAPSHOT_BUFFER_SIZE);
}
const unsigned int Application::getFrameIndex() const { 
	return m_frameInd.load(); 
}
const unsigned int Application::getSnapshotBufferIndex() const {
	return m_snapshotBufInd.load();
}
