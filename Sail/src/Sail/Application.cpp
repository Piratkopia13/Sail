#include "pch.h"
#include "Application.h"
#include "events/WindowResizeEvent.h"
#include "../../SPLASH/src/game/events/TextInputEvent.h" // ONLY 2 BITCH
#include "KeyCodes.h"
#include "graphics/geometry/Transform.h"
#include "Sail/graphics/Scene.h"


Application* Application::s_instance = nullptr;
std::atomic_uint Application::s_queuedUpdates = 0;
std::atomic_uint Application::s_updateRunning = 0;
std::atomic_bool Application::s_isRunning = true;


Application::Application(int windowWidth, int windowHeight, const char* windowTitle, HINSTANCE hInstance, API api) {

	// Set up instance if not set
	if (s_instance) {
		Logger::Error("Only one application can exist!");
		return;
	}
	s_instance = this;

	// Set up thread pool with two times as many threads as logical cores, or four threads if the CPU only has one core;
	// Note: this value might need future optimization
	unsigned int poolSize = std::max<unsigned int>(4, (2 * std::thread::hardware_concurrency()));
	m_threadPool = std::unique_ptr<ctpl::thread_pool>(SAIL_NEW ctpl::thread_pool(poolSize));

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

	m_nodeSystem = std::make_unique<NodeSystem>();
}

Application::~Application() {
	delete Input::GetInstance();
}


// CAUTION: HERE BE DRAGONS!
// Moving around function calls in this function is likely to cause bugs and crashes
int Application::startGameLoop() {
	MSG msg = { 0 };
	m_fps = 0;
	// Start delta timer
	m_timer.startTimer();
	const INT64 startTime = m_timer.getStartTime();

	float currentTime = m_timer.getTimeSince<float>(startTime);
	float newTime = 0.0f;
	float delta = 0.0f;
	float accumulator = 0.0f;
	float secCounter = 0.0f;
	float elapsedTime = 0.0f;
	UINT frameCounter = 0;

	// Render loop, each iteration of it results in one rendered frame
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_KEYDOWN) {
				dispatchEvent(TextInputEvent(msg));
			}
		
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
			newTime = m_timer.getTimeSince<float>(startTime);
			delta = newTime - currentTime;
			currentTime = newTime;

			// Will slow the game down if the CPU can't keep up with the TICKRATE
			delta = std::min(delta, 4 * TIMESTEP);

			// Update fps counter
			secCounter += delta;
			accumulator += delta;
			frameCounter++;
			if (secCounter >= 1.0) {
				m_fps = frameCounter;
				frameCounter = 0;
				secCounter = 0.0;
			}

			// alpha value used for the interpolation later on
			float alpha = accumulator/TIMESTEP;

			// Queue multiple updates if the game has fallen behind to make sure that it catches back up to the current time.
			while (accumulator >= TIMESTEP) {
				accumulator -= TIMESTEP;
				s_queuedUpdates++;
			}

			// Update mouse deltas
			Input::GetInstance()->beginFrame();

			//UPDATES ALL CURRENTLY-WORKING AUDIO FUNCTIONALITY (TL;DR - Press '9' and '0')
			Application::getAudioManager()->updateAudio();

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
			// NOTE: player movement is processed in update() except for mouse movement which is processed here
			processInput(delta);

			// Don't create a new update thread if another one is already running the update loop
			if (s_updateRunning == 0) {
				s_updateRunning = 1;
				// Run update(s) in a separate thread
				m_threadPool->push([this](int id) {
					while (s_queuedUpdates > 0 && s_isRunning) {
						s_queuedUpdates--;
						Scene::IncrementCurrentUpdateIndex();
						update(TIMESTEP);
					}
					s_updateRunning = 0;
					});
			}

			// Render
			Scene::UpdateCurrentRenderIndex();

			render(delta, alpha);
			//render(delta, 1.0f); // disable interpolation

			// Reset just pressed keys
			Input::GetInstance()->endFrame();
		}
	}
	s_isRunning = false;
	m_threadPool->stop();
	return (int)msg.wParam;
}

std::string Application::getPlatformName() {
	return std::string(SAIL_PLATFORM);
}

Application* Application::getInstance() {
	if (!s_instance)
		Logger::Error("Application instance not set, you need to initialize the class which inherits from Application before calling getInstance().");
	return s_instance;
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
MemoryManager& Application::getMemoryManager() {
	return m_memoryManager;
}
Audio* Application::getAudioManager() {
	return &m_audioManager;
}
NodeSystem* Application::getNodeSystem() {
	return m_nodeSystem.get();
}
StateStorage& Application::getStateStorage() {
	return this->m_stateStorage;
}
const UINT Application::getFPS() const {
	return m_fps;
}


