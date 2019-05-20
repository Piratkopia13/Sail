#include "GameState.h"

GameState::GameState(StateStack& stack)
: State(stack)
//, m_cam(20.f, 20.f, 0.1f, 5000.f)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
, m_flyCam(true)
, m_fpsText(nullptr)
, m_debugCamText(nullptr)
{

	// Get the Application instance
	m_app = Application::getInstance();
	//m_scene = std::make_unique<Scene>(AABB(glm::vec3(-100.f, -100.f, -100.f), glm::vec3(100.f, 100.f, 100.f)));

	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	//Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");

	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 1.7f, -1.4f));
	m_camController.lookAt(glm::vec3(0.f));
	//m_cam.setPosition(glm::vec3(0.f, 0.f, -50.f));
	//m_cam.setDirection(glm::vec3(0.f, 0.f, 1.f));
	
	// Set up the scene
	//m_scene->addSkybox(L"skybox_space_512.dds");
	// Add a directional light
	glm::vec3 color(1.0f, 1.0f, 1.0f);
 	glm::vec3 direction(0.4f, -0.6f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));


	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	//auto* shader = &m_app->getResourceManager().getShaderSet<DeferredGeometryShader>();
	auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();

	m_cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), shader->getPipeline());
	m_cubeModel->getMesh(0)->getMaterial()->setDiffuseTexture("missing.tga");
	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(5.f), shader->getPipeline());

	m_scene.setLightSetup(&m_lights);

	auto e = Entity::Create();
	e->addComponent<ModelComponent>(m_cubeModel.get());
	Transform& transform = e->addComponent<TransformComponent>()->getTransform();
	transform.setRotations(glm::vec3(0.f, 0.f, 1.07f));
	transform.setTranslation(glm::vec3(1.2f, 1.f, 1.f));
	m_scene.addEntity(MOVE(e));

	e = Entity::Create();
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setTranslation(glm::vec3(0.f, 1.f, 0.f));
	m_scene.addEntity(MOVE(e));

	e = Entity::Create();
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setTranslation(glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(MOVE(e));

	Model* fbxModel = &m_app->getResourceManager().getModel("box.fbx", shader->getPipeline());
	e = Entity::Create();
	e->addComponent<ModelComponent>(fbxModel);
	fbxModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	fbxModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	e->addComponent<TransformComponent>()->getTransform().setTranslation(glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(MOVE(e));

	//e = Entity::Create();
	//auto* textComp = e->addComponent<TextComponent>();
	//m_fpsText = textComp->addText(Text::Create(L"FPSText"));
	//m_debugCamText = textComp->addText(Text::Create(L"CamText"));
	//m_scene.addEntity(MOVE(e));

	//// Set up HUD texts
	//if (m_debugCamText)
	//	m_debugCamText->setPosition(glm::vec2(0.f, 20.f));

}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {

	//std::cout << Utils::toStr(Input::GetMousePosition()) << std::endl;
	//std::cout << Input::IsKeyPressed(SAIL_KEY_CONTROL) << std::endl;
	/*for (int i = 0; i < 256; i++) {
		if (Input::IsKeyPressed(i)) {
			std::cout << "pressed: " << i << std::endl;
		}
	}*/
	/*if (Input::WasKeyJustPressed(SAIL_KEY_CONTROL))
		std::cout << "RETURN" << std::endl;
	if (Input::WasMouseButtonJustPressed(SAIL_MOUSE_BUTTON_1))
		std::cout << "LEFT MOUSE" << std::endl;*/
	//std::cout << Input::IsMouseButtonPressed(SAIL_MOUSE_BUTTON_1) << " " << Input::IsMouseButtonPressed(SAIL_MOUSE_BUTTON_2) << " " << Input::IsMouseButtonPressed(SAIL_MOUSE_BUTTON_3) << " " << Input::IsMouseButtonPressed(SAIL_MOUSE_BUTTON_4) << " " << Input::IsMouseButtonPressed(SAIL_MOUSE_BUTTON_5) << std::endl;


#ifdef _DEBUG
	// Toggle camera controller on 'F' key or 'Y' btn
	if (Input::WasKeyJustPressed(SAIL_KEY_F))
		m_flyCam = !m_flyCam;
	// Add point light at camera pos
	if (Input::WasKeyJustPressed(SAIL_KEY_E)) {
		PointLight pl;
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(m_cam.getPosition());
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		m_lights.addPointLight(pl);
	}
#endif

	if (Input::IsKeyPressed(SAIL_KEY_G)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);;
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Update the camera controller from input devices
	if (m_flyCam)
		m_camController.update(dt);

	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadShader<MaterialShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
	}


	return true;
}

void GameState::onEvent(Event& event) {
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<WindowResizeEvent>(event, FUNC(&GameState::onResize));

	// Forward events
	m_scene.onEvent(event);
}

bool GameState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	//m_scene->resize(width, height);
	return true;
}

bool GameState::update(float dt) {

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());
	// Update HUD texts
	if (m_fpsText)
		m_fpsText->setText(L"FPS: " + fpsStr);

	auto& camPos = m_cam.getPosition();
	if (m_debugCamText)
		m_debugCamText->setText(L"Camera @ " + Utils::toWStr(camPos) + L"\n" + 
			L"GPU memory usage: " + std::to_wstring(m_app->getAPI()->getMemoryUsage()) + L"/" + std::to_wstring(m_app->getAPI()->getMemoryBudget()) + L"mb");

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | FPS: " + std::to_string(m_app->getFPS()));

	return true;
}

// Renders the state
bool GameState::render(float dt) {

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

	// Draw HUD
	//m_scene->drawHUD();

	return true;
}
