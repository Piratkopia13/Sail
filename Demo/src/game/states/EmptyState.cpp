#include "EmptyState.h"
#include "imgui.h"


EmptyState::EmptyState(StateStack& stack)
	: State(stack)
{
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();
	
}

EmptyState::~EmptyState() { }

// Process input for the state
bool EmptyState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

	return true;
}

bool EmptyState::update(float dt) {
	SAIL_PROFILE_FUNCTION();

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

#ifdef _DEBUG
	std::string config = "Debug";
#else
	std::string config = "Release";
#endif
	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " " + config + " | FPS: " + std::to_string(m_app->getFPS()));

	return true;
}

// Renders the state
bool EmptyState::render(float dt) {
	SAIL_PROFILE_FUNCTION();

	return true;
}

bool EmptyState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();
	
	return false;
}
