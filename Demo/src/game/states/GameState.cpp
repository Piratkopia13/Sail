#include "GameState.h"
#include "imgui.h"

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
	m_planeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));

	m_scene.setLightSetup(&m_lights);

	

	auto e = Entity::Create("Cube1");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(-4.f, 1.f, -2.f));
	m_scene.addEntity(e);

	e = Entity::Create("Plane");
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(e);

	e = Entity::Create("Cube0");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	TransformComponent* transform = e->addComponent<TransformComponent>(glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));
	m_scene.addEntity(e);

	Model* fbxModel = &m_app->getResourceManager().getModel("box.fbx", shader->getPipeline());
	m_testEntity = Entity::Create("thisIsAName");
	m_testEntity->addComponent<ModelComponent>(fbxModel);
	fbxModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	fbxModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	m_testEntity->addComponent<TransformComponent>(glm::vec3(-1.f, 0.f, 0.f), m_testEntity->getComponent<TransformComponent>());
	m_testEntity->setName("MovingCube");
	m_scene.addEntity(m_testEntity);
	e->getComponent<TransformComponent>()->setParent(
		m_testEntity->getComponent<TransformComponent>());


	createTransformTest();







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


	if (Input::WasKeyJustPressed(SAIL_KEY_1)) {
		Logger::Log("setting parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformComponent>());
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_2)) {
		Logger::Log("removing parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->removeParent();
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

bool GameState::onEvent(Event& event) {
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&GameState::onResize));

	// Forward events
	m_scene.onEvent(event);

	return true;
}

bool GameState::onResize(WindowResizeEvent& event) {
	//Logger::Log("resize event!");
	m_cam.resize(event.getWidth(), event.getHeight());
	//m_scene->resize(width, height);
	return true;
}

void GameState::createTransformTest() {



	auto e = Entity::Create("CubeRoot");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(10.f, 0.f, 10.f));
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("Cube1st");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[0]->getComponent<TransformComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("Cube1st");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[1]->getComponent<TransformComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);












}

void GameState::updateTransformTest(const float dt) {

	static float size = 1;
	static float change = 1.0f;

	for (Entity::SPtr item : m_transformTestEntities) {
		item->getComponent<TransformComponent>()->rotateAroundY(dt * 3.14f);
		item->getComponent<TransformComponent>()->setScale(size);
		item->getComponent<TransformComponent>()->setTranslation(size * 3, 1.0f, size * 3);
		
	}
	m_transformTestEntities[0]->getComponent<TransformComponent>()->translate(2.0f, 0.0f, 2.0f);

	size += change * dt;
	if (size > 1.2f || size < 0.7f)
		change *= -1.0f;





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

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	counter += dt * 4;
	if (m_testEntity) {
		m_testEntity->getComponent<TransformComponent>()->setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		m_testEntity->getComponent<TransformComponent>()->setRotations(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));

		updateTransformTest(dt);
	}

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

bool GameState::renderImgui(float dt) {
	ImGui::ShowDemoWindow();
	return false;
}
