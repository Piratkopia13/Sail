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
	auto cubeMesh = MeshFactory::Cube::Create(glm::vec3(0.5f));
	auto planeMesh = MeshFactory::Plane::Create(glm::vec2(50.f), glm::vec2(30.0f));
	auto sphereMesh = m_app->getResourceManager().getMesh("sphere.fbx");

	// Create entities

	// Lights
	{
		// Add a directional light
		auto e = m_scene.createEntity("Directional light");
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		glm::vec3 direction(0.4f, -0.2f, 1.0f);
		direction = glm::normalize(direction);
		e.addComponent<DirectionalLightComponent>(color, direction);

		// Add four point lights
		e = m_scene.createEntity("Point light 1");
		auto* pl = &e.addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(-4.0f, 0.1f, -4.0f));

		e = m_scene.createEntity("Point light 2");
		pl = &e.addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(-4.0f, 0.1f, 4.0f));

		e = m_scene.createEntity("Point light 3");
		pl = &e.addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(4.0f, 0.1f, 4.0f));

		e = m_scene.createEntity("Point light 4");
		pl = &e.addComponent<PointLightComponent>();
		pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl->setPosition(glm::vec3(4.0f, 0.1f, -4.0f));
	}
	{
		auto e = m_scene.createEntity("Static cube");
		e.addComponent<MeshComponent>(cubeMesh);
		e.addComponent<TransformComponent>(glm::vec3(-4.f, 1.f, -2.f));
		auto mat = std::make_shared<PhongMaterial>();
		mat->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));
		e.addComponent<MaterialComponent>(mat);
	}

	{
		auto e = m_scene.createEntity("Floor");
		e.addComponent<MeshComponent>(planeMesh);
		e.addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
		auto mat = std::make_shared<PhongMaterial>();
		mat->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
		mat->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
		mat->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
		e.addComponent<MaterialComponent>(mat);
	}

	Entity parentEntity;
	{
		parentEntity = m_scene.createEntity("Clingy cube");
		parentEntity.addComponent<MeshComponent>(cubeMesh);
		parentEntity.addComponent<TransformComponent>(glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));
		parentEntity.addComponent<MaterialComponent>(std::make_shared<PhongMaterial>());
	}
	{
		// Add some cubes which are connected through parenting
		m_texturedCubeEntity = m_scene.createEntity("Textured parent cube");
		m_texturedCubeEntity.addComponent<MeshComponent>(sphereMesh);
		m_texturedCubeEntity.addComponent<TransformComponent>(glm::vec3(-1.f, 2.f, 0.f));
		auto mat = std::make_shared<PhongMaterial>();
		mat->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
		mat->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
		mat->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
		m_texturedCubeEntity.addComponent<MaterialComponent>(mat);
		m_texturedCubeEntity.setName("MovingCube");

		parentEntity.addComponent<RelationshipComponent>().parent = m_texturedCubeEntity;
	}
	{
		auto e = m_scene.createEntity("CubeRoot");
		e.addComponent<MeshComponent>(cubeMesh);
		e.addComponent<TransformComponent>(glm::vec3(10.f, 0.f, 10.f));
		e.addComponent<MaterialComponent>(std::make_shared<PhongMaterial>());
		m_transformTestEntities.push_back(e);
	}
	{
		auto e = m_scene.createEntity("CubeChild");
		e.addComponent<MeshComponent>(cubeMesh);
		e.addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f));
		e.addComponent<MaterialComponent>(std::make_shared<PhongMaterial>());
		m_transformTestEntities.push_back(e);
		e.addComponent<RelationshipComponent>().parent = m_transformTestEntities[0];
	}
	{
		auto e = m_scene.createEntity("CubeChildChild");
		e.addComponent<MeshComponent>(cubeMesh);
		e.addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f));
		e.addComponent<MaterialComponent>(std::make_shared<PhongMaterial>());
		e.addComponent<RelationshipComponent>().parent = m_transformTestEntities[1];
		m_transformTestEntities.push_back(e);
	}
	
	// Random cube maze
	const unsigned int mazeStart = 5;
	const unsigned int mazeSize = 20;
	const float wallSize = 1.1f;
	for (unsigned int x = 0; x < mazeSize; x++) {
		for (unsigned int y = 0; y < mazeSize; y++) {
			if (Utils::rnd() > 0.5f)
				continue;

			auto e = m_scene.createEntity();
			e.addComponent<MeshComponent>(cubeMesh);
			e.addComponent<TransformComponent>(glm::vec3(x * wallSize + mazeStart, 0.5f, y * wallSize + mazeStart));
			e.addComponent<MaterialComponent>(std::make_shared<PhongMaterial>());
		}
	}
}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

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
		m_texturedCubeEntity.getComponent<TransformComponent>().setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		m_texturedCubeEntity.getComponent<TransformComponent>().setRotations(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));

		// Move the three parented cubes with identical translation, rotations and scale to show how parenting affects transforms
		for (Entity& item : m_transformTestEntities) {
			item.getComponent<TransformComponent>().rotateAroundY(dt * 1.0f);
			item.getComponent<TransformComponent>().setScale(size);
			item.getComponent<TransformComponent>().setTranslation(size * 3, 1.0f, size * 3);
		}
		m_transformTestEntities[0].getComponent<TransformComponent>().translate(2.0f, 0.0f, 2.0f);

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
