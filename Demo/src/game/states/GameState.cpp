#include "GameState.h"
#include "imgui.h"
#include "Sail/debug/Instrumentor.h"
#include "Sail/graphics/material/PhongMaterial.h"

GameState::GameState(StateStack& stack)
: State(stack)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
{
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();

	// Textures needs to be loaded before they can be used
	// TODO: automatically load textures when needed so the following can be removed
	/*Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");*/

	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 4.7f, 7.4f));
	m_camController.lookAt(glm::vec3(0.f));

	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	// Create/load models
	auto cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f));
	auto planeModel = ModelFactory::PlaneModel::Create(glm::vec2(50.f), glm::vec2(30.0f));
	auto fbxModel = m_app->getResourceManager().getModel("sphere.fbx");

	// Create entities

	// Lights
	{
		// Add a directional light
		auto e = Entity::Create("Directional light");
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		glm::vec3 direction(0.4f, -0.2f, 1.0f);
		direction = glm::normalize(direction);
		e->addComponent<DirectionalLightComponent>(color, direction);
		m_scene.addEntity(e);

		// Add four point lights
		e = Entity::Create("Point light 1");
		auto& pl = e->addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(-4.0f, 0.1f, -4.0f));
		m_scene.addEntity(e);

		e = Entity::Create("Point light 2");
		pl = e->addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(-4.0f, 0.1f, 4.0f));
		m_scene.addEntity(e);

		e = Entity::Create("Point light 3");
		pl = e->addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(4.0f, 0.1f, 4.0f));
		m_scene.addEntity(e);

		e = Entity::Create("Point light 4");
		pl = e->addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(4.0f, 0.1f, -4.0f));
		m_scene.addEntity(e);
	}
	{
		auto e = Entity::Create("Static cube");
		e->addComponent<ModelComponent>(cubeModel);
		e->addComponent<TransformComponent>(glm::vec3(-4.f, 1.f, -2.f));
		auto mat = e->addComponent<MaterialComponent<PhongMaterial>>();
		mat->get()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));
		m_scene.addEntity(e);
	}

	{
		auto e = Entity::Create("Floor");
		e->addComponent<ModelComponent>(planeModel);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
		auto mat = e->addComponent<MaterialComponent<PhongMaterial>>();
		mat->get()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
		mat->get()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
		mat->get()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
		m_scene.addEntity(e);
	}

	Entity::SPtr parentEntity;
	{
		parentEntity = Entity::Create("Clingy cube");
		parentEntity->addComponent<ModelComponent>(cubeModel);
		parentEntity->addComponent<TransformComponent>(glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));
		parentEntity->addComponent<MaterialComponent<PhongMaterial>>();
		m_scene.addEntity(parentEntity);
	}
	{
		// Add some cubes which are connected through parenting
		m_texturedCubeEntity = Entity::Create("Textured parent cube");
		m_texturedCubeEntity->addComponent<ModelComponent>(fbxModel);
		m_texturedCubeEntity->addComponent<TransformComponent>(glm::vec3(-1.f, 2.f, 0.f), m_texturedCubeEntity->getComponent<TransformComponent>().get());
		auto mat = m_texturedCubeEntity->addComponent<MaterialComponent<PhongMaterial>>();
		mat->get()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
		mat->get()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
		mat->get()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
		m_texturedCubeEntity->setName("MovingCube");
		m_scene.addEntity(m_texturedCubeEntity);
		parentEntity->getComponent<TransformComponent>()->setParent(m_texturedCubeEntity->getComponent<TransformComponent>().get());
	}
	{
		auto e = Entity::Create("CubeRoot");
		e->addComponent<ModelComponent>(cubeModel);
		e->addComponent<TransformComponent>(glm::vec3(10.f, 0.f, 10.f));
		e->addComponent<MaterialComponent<PhongMaterial>>();
		m_scene.addEntity(e);
		m_transformTestEntities.push_back(e);
	}
	{
		auto e = Entity::Create("CubeChild");
		e->addComponent<ModelComponent>(cubeModel);
		e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[0]->getComponent<TransformComponent>().get());
		e->addComponent<MaterialComponent<PhongMaterial>>();
		m_scene.addEntity(e);
		m_transformTestEntities.push_back(e);
	}
	{
		auto e = Entity::Create("CubeChildChild");
		e->addComponent<ModelComponent>(cubeModel);
		e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[1]->getComponent<TransformComponent>().get());
		e->addComponent<MaterialComponent<PhongMaterial>>();
		m_scene.addEntity(e);
		m_transformTestEntities.push_back(e);
	}
	
	// Random cube maze
	const unsigned int mazeStart = 5;
	const unsigned int mazeSize = 20;
	const float wallSize = 1.1f;
	for (unsigned int x = 0; x < mazeSize; x++) {
		for (unsigned int y = 0; y < mazeSize; y++) {
			/*if (Utils::rnd() > 0.5f)
				continue;*/

			auto e = Entity::Create();
			e->addComponent<ModelComponent>(cubeModel);
			e->addComponent<TransformComponent>(glm::vec3(x * wallSize + mazeStart, 0.5f, y * wallSize + mazeStart));
			e->addComponent<MaterialComponent<PhongMaterial>>();
			m_scene.addEntity(e);
		}
	}
}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

#ifdef _DEBUG
	if (Input::WasKeyJustPressed(SAIL_KEY_1)) {
		Logger::Log("Setting parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformComponent>().get());
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_2)) {
		Logger::Log("Removing parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->removeParent();
	}
#endif

	// Update the camera controller from input devices
	m_camController.update(dt);

	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadAllShaders();
	}

	return true;
}

bool GameState::update(float dt) {
	SAIL_PROFILE_FUNCTION();

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1;
	static float change = 0.4f;
	
	counter += dt * 2;
	if (m_texturedCubeEntity) {
		// Move the cubes around
		m_texturedCubeEntity->getComponent<TransformComponent>()->setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		m_texturedCubeEntity->getComponent<TransformComponent>()->setRotations(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));

		// Move the three parented cubes with identical translation, rotations and scale to show how parenting affects transforms
		for (Entity::SPtr item : m_transformTestEntities) {
			item->getComponent<TransformComponent>()->rotateAroundY(dt * 1.0f);
			item->getComponent<TransformComponent>()->setScale(size);
			item->getComponent<TransformComponent>()->setTranslation(size * 3, 1.0f, size * 3);
		}
		m_transformTestEntities[0]->getComponent<TransformComponent>()->translate(2.0f, 0.0f, 2.0f);

		size += change * dt;
		if (size > 1.2f || size < 0.7f)
			change *= -1.0f;
	}

	return true;
}

// Renders the state
bool GameState::render(float dt) {
	SAIL_PROFILE_FUNCTION();

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

	return true;
}

bool GameState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	return false;
}
