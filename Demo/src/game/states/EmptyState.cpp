#include "EmptyState.h"
#include "imgui.h"


EmptyState::EmptyState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.01f, 5000.f)
	, m_camController(&m_cam) {
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();

	m_cam.setPosition({ 0.f, 0.f, -1.f });
	// Make sure the following can be removed

	m_forwardRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	m_mesh = MeshFactory::Plane::Create(glm::vec2(10.0f), glm::vec2(5.0f));
	//m_model2 = ModelFactory::PlaneModel::Create(glm::vec2(0.5f), glm::vec2(50.0f));
	//m_model2 = m_app->getResourceManager().getModel("box.fbx");

	//m_material.setColor({0.8f, 0.2f, 0.2f, 1.0f});
	/*m_material2.setDiffuseTexture("pbr/pavingStones/albedo.tga");
	m_material2.setNormalTexture("pbr/pavingStones/normal.tga");*/
	//m_material2.setDiffuseTexture("pbr/ice/albedo.tga");
	//m_material2.setNormalTexture("pbr/ice/normal.tga");

	m_pbrMaterial.setColor({1.f, 1.f, 1.f, 1.f});
	
	//m_material2.setColor({ 0.2f, 0.8f, 0.2f, 1.0f });
	//m_material2.setDiffuseTexture("pbr/cerberus/Cerberus_A.tga");

	DirectionalLightComponent dlComp;
	m_lightSetup.setDirectionalLight(&dlComp);

	m_forwardRenderer->setLightSetup(&m_lightSetup);


	{
		// Add a directional light
		auto e = Entity::Create("Directional light");
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		glm::vec3 direction(0.4f, -0.2f, 1.0f);
		direction = glm::normalize(direction);
		e->addComponent<DirectionalLightComponent>(color, direction);
		m_scene.addEntity(e);
	}

	{
		auto e = Entity::Create("Static cube");
		e->addComponent<MeshComponent>(m_mesh2);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
		auto mat = e->addComponent<MaterialComponent<PBRMaterial>>();
		mat->get()->setRoughnessScale(0.f);
		mat->get()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));
		mat->get()->setAlbedoTexture("pbr/pavingStones/albedo.tga");
		m_scene.addEntity(e);
	}

	{
		auto e = Entity::Create("Static cube");
		e->addComponent<MeshComponent>(m_mesh2);
		e->addComponent<TransformComponent>(glm::vec3(0.f, -2.f, 0.f));
		auto mat = e->addComponent<MaterialComponent<PBRMaterial>>();
		mat->get()->setRoughnessScale(0.f);
		mat->get()->setColor(glm::vec4(0.8f, 0.1f, 0.4f, 1.0f));
		m_scene.addEntity(e);
	}
}

EmptyState::~EmptyState() { }

// Process input for the state
bool EmptyState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

	m_camController.update(dt);

	if (Input::WasKeyJustPressed(SAIL_KEY_G)) {
		DirectionalLightComponent dlComp;
		dlComp.setDirection(m_cam.getDirection());
		m_lightSetup.setDirectionalLight(&dlComp);
	}

	if (Input::WasKeyJustPressed(SAIL_KEY_F)) {
		PointLightComponent plComp;
		plComp.setColor(Utils::getRandomColor());
		plComp.setPosition(m_cam.getPosition());
		m_lightSetup.addPointLight(&plComp);
	}

	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadAllShaders();
	}

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

	m_scene.draw(m_cam);

	//// Draw the scene
	//m_forwardRenderer->begin(&m_cam, &m_environment);

	//static glm::mat4 transform(1.f);
	//transform = glm::rotate(transform, dt * 0.2f, glm::vec3(0.f, 1.0f, 0.f));

	//static float counter = 0.f;
	//counter += dt * 0.0001f;
	////static glm::mat4 transform2 = glm::identity<glm::mat4>();
	////transform2 = glm::translate(transform2, glm::vec3(glm::sin(counter), 0.f, 0.f));
	//glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), glm::vec3(4.f, 0.f, 0.f));
	////static glm::mat4 transform2(1.f);
	////transform2 = glm::rotate(transform2, dt * 0.1f, glm::vec3(0.f, 1.0f, 0.f));


	//m_forwardRenderer->submit(m_model2.get(), &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PBRMaterialShader), &m_pbrMaterial, transform2);
	////m_forwardRenderer->submit(m_model2.get(), &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PhongMaterialShader), &m_material2, transform);

	//m_forwardRenderer->end();
	//m_forwardRenderer->present(Renderer::Default);

	return true;
}

bool EmptyState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();
	
	ImGui::ShowDemoWindow();

	return false;
}
