#include "EmptyState.h"
#include "imgui.h"


EmptyState::EmptyState(StateStack& stack)
	: State(stack)
{
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();


	// Make sure the following can be removed

	m_forwardRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	m_model = ModelFactory::PlaneModel::Create(glm::vec2(0.3f), glm::vec2(30.0f));
	
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

	// Draw the scene
	m_forwardRenderer->begin(nullptr, nullptr);

	m_forwardRenderer->submit(m_model.get(), &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PhongMaterialShader), nullptr, glm::identity<glm::mat4>());

	m_forwardRenderer->end();
	m_forwardRenderer->present(Renderer::Default);

	return true;
}

bool EmptyState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();
	
	return false;
}
