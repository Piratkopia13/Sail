#include "EmptyState.h"
#include "imgui.h"


EmptyState::EmptyState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
	, m_camController(&m_cam)
{
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();

	m_cam.setPosition({0.f, 0.f, -1.f});
	// Make sure the following can be removed

	m_forwardRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	m_model = ModelFactory::PlaneModel::Create(glm::vec2(0.3f), glm::vec2(1.0f));
	//m_model2 = ModelFactory::PlaneModel::Create(glm::vec2(0.5f), glm::vec2(50.0f));
	m_model2 = m_app->getResourceManager().getModel("box.fbx");

	m_material.setDiffuseTexture("pbr/pavingStones/albedo.tga");
	m_material2.setDiffuseTexture("pbr/cerberus/Cerberus_A.tga");
}

EmptyState::~EmptyState() { }

// Process input for the state
bool EmptyState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

	m_camController.update(dt);

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
	m_forwardRenderer->begin(&m_cam, nullptr);

	static glm::mat4 transform = glm::identity<glm::mat4>();
	transform = glm::rotate(transform, dt * 1.f, glm::vec3(1.f, 1.0f, 0.f));

	static float counter = 0.f;
	counter += dt * 0.0001f;
	//static glm::mat4 transform2 = glm::identity<glm::mat4>();
	//transform2 = glm::translate(transform2, glm::vec3(glm::sin(counter), 0.f, 0.f));
	glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f));

	m_forwardRenderer->submit(m_model.get(), &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PhongMaterialShader), &m_material, transform2);
	m_forwardRenderer->submit(m_model2.get(), &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PhongMaterialShader), &m_material2, transform);

	m_forwardRenderer->end();
	m_forwardRenderer->present(Renderer::Default);

	return true;
}

bool EmptyState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();
	
	return false;
}
