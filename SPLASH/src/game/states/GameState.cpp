#include "GameState.h"
#include "imgui.h"

GameState::GameState(StateStack& stack)
: State(stack)
//, m_cam(20.f, 20.f, 0.1f, 5000.f)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
{

	// Get the Application instance
	m_app = Application::getInstance();
	//m_scene = std::make_unique<Scene>(AABB(glm::vec3(-100.f, -100.f, -100.f), glm::vec3(100.f, 100.f, 100.f)));

	// Textures needs to be loaded before they can be used
	// TODO: automatically load textures when needed so the following can be removed
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");

	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 4.7f, 7.4f));
	m_camController.lookAt(glm::vec3(0.f));
	
	// Add a directional light
	glm::vec3 color(1.0f, 1.0f, 1.0f);
 	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));
	// Add four point lights
	{
		PointLight pl;
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(-4.0f, 0.1f, -4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(-4.0f, 0.1f, 4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(4.0f, 0.1f, 4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(4.0f, 0.1f, -4.0f));
		m_lights.addPointLight(pl);
	}

	// Set up the scene
	//m_scene->addSkybox(L"skybox_space_512.dds"); //TODO
	m_scene.setLightSetup(&m_lights);

	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();

	// Create/load models
	m_cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), shader);
	m_cubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));
	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(5.f), shader, glm::vec2(3.0f));
	m_planeModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	m_planeModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	m_planeModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
	
	Model* fbxModel = &m_app->getResourceManager().getModel("sphere.fbx", shader);
	fbxModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	fbxModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	fbxModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");


	// Create entities
	auto e = Entity::Create("Static cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(-4.f, 1.f, -2.f));
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(-4.f, 1.f, -2.f));
	m_scene.addEntity(e);

	e = Entity::Create("Floor");
	e->addComponent<ModelComponent>(m_planeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(e);

	// Add some cubes which are connected through parenting
	m_texturedCubeEntity = Entity::Create("Textured parent cube");
	m_texturedCubeEntity->addComponent<ModelComponent>(fbxModel);
	//m_texturedCubeEntity->addComponent<TransformComponent>(glm::vec3(-1.f, 2.f, 0.f), m_texturedCubeEntity->getComponent<TransformComponent>());
	//m_texturedCubeEntity->addComponent<TransformDataComponent>(glm::vec3(-1.f, 2.f, 0.f), m_texturedCubeEntity->getComponent<TransformDataComponent>());
	//m_texturedCubeEntity->addComponent<TransformMatrixComponent>(m_texturedCubeEntity->getComponent<TransformMatrixComponent>());
	m_texturedCubeEntity->addComponent<TransformMatrixComponent>();
	m_texturedCubeEntity->addComponent<TransformDataComponent>(m_texturedCubeEntity->getComponent<TransformMatrixComponent>(), glm::vec3(-1.f, 2.f, 0.f));
	m_texturedCubeEntity->setName("MovingCube");
	m_scene.addEntity(m_texturedCubeEntity);

	e = Entity::Create("Clingy cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));



	//e->getComponent<TransformComponent>()->setParent(m_texturedCubeEntity->getComponent<TransformComponent>());
	//e->getComponent<TransformDataComponent>()->setParent(m_texturedCubeEntity->getComponent<TransformDataComponent>());
	e->getComponent<TransformDataComponent>()->setParent(m_texturedCubeEntity->getComponent<TransformDataComponent>());
	m_scene.addEntity(e);



	e = Entity::Create("CubeRoot");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(10.f, 0.f, 10.f));
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(10.f, 0.f, 10.f));
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("CubeChild");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[0]->getComponent<TransformComponent>());
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(1.f, 1.f, 1.f));
	//e->getComponent<TransformDataComponent>()->setParent(m_transformTestEntities[0]->getComponent<TransformDataComponent>());
	e->getComponent<TransformDataComponent>()->setParent(m_transformTestEntities[0]->getComponent<TransformDataComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("CubeChildChild");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	//e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[1]->getComponent<TransformComponent>());
	e->addComponent<TransformMatrixComponent>();
	e->addComponent<TransformDataComponent>(e->getComponent<TransformMatrixComponent>(), glm::vec3(1.f, 1.f, 1.f));
	//e->getComponent<TransformDataComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformDataComponent>());
	e->getComponent<TransformDataComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformDataComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);






}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {

#ifdef _DEBUG
	// Add point light at camera pos
	if (Input::WasKeyJustPressed(SAIL_KEY_E)) {
		PointLight pl;
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(m_cam.getPosition());
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		m_lights.addPointLight(pl);
	}

	if (Input::WasKeyJustPressed(SAIL_KEY_1)) {
		Logger::Log("Setting parent");
		//m_transformTestEntities[2]->getComponent<TransformComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformComponent>());
		//m_transformTestEntities[2]->getComponent<TransformMatrixComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformMatrixComponent>());
		m_transformTestEntities[2]->getComponent<TransformDataComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformDataComponent>());
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_2)) {
		Logger::Log("Removing parent");
		//m_transformTestEntities[2]->getComponent<TransformComponent>()->removeParent();
		//m_transformTestEntities[2]->getComponent<TransformMatrixComponent>()->removeParent();
		m_transformTestEntities[2]->getComponent<TransformDataComponent>()->removeParent();
	}
#endif

	if (Input::IsKeyPressed(SAIL_KEY_G)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);;
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Update the camera controller from input devices
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
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool GameState::update(float dt) {

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1;
	static float change = 0.4f;
	
	counter += dt * 2;
	if (m_texturedCubeEntity) {
		// Move the cubes around
		//m_texturedCubeEntity->getComponent<TransformComponent>()->setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		//m_texturedCubeEntity->getComponent<TransformComponent>()->setRotations(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));
		m_texturedCubeEntity->getComponent<TransformDataComponent>()->setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		m_texturedCubeEntity->getComponent<TransformDataComponent>()->setRotation(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));
		//m_texturedCubeEntity->getComponent<TransformMatrixComponent>()->treeNeedsUpdating();

		// Move the three parented cubes with identical translation, rotations and scale to show how parenting affects transforms
		for (Entity::SPtr item : m_transformTestEntities) {
			/*item->getComponent<TransformComponent>()->rotateAroundY(dt * 1.0f);
			item->getComponent<TransformComponent>()->setScale(size);
			item->getComponent<TransformComponent>()->setTranslation(size * 3, 1.0f, size * 3);*/
			item->getComponent<TransformDataComponent>()->rotateAroundY(dt * 1.0f);
			item->getComponent<TransformDataComponent>()->setScale(size);
			item->getComponent<TransformDataComponent>()->setTranslation(size * 3, 1.0f, size * 3);
			//item->getComponent<TransformMatrixComponent>()->treeNeedsUpdating();
		}
		//m_transformTestEntities[0]->getComponent<TransformComponent>()->translate(2.0f, 0.0f, 2.0f);
		m_transformTestEntities[0]->getComponent<TransformDataComponent>()->translate(2.0f, 0.0f, 2.0f);
		//m_transformTestEntities[0]->getComponent<TransformMatrixComponent>()->treeNeedsUpdating();

		size += change * dt;
		if (size > 1.2f || size < 0.7f)
			change *= -1.0f;
	}

	return true;
}

// Renders the state
bool GameState::render(float dt) {

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

	return true;
}

bool GameState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	return false;
}
