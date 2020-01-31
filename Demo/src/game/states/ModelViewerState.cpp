#include "ModelViewerState.h"
#include "imgui.h"
#include "Sail/debug/Instrumentor.h"

ModelViewerState::ModelViewerState(StateStack& stack)
: State(stack)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
{
	SAIL_PROFILE_FUNCTION();

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
	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(50.f), shader, glm::vec2(30.0f));
	m_planeModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	m_planeModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	m_planeModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");
	
	Model* fbxModel = &m_app->getResourceManager().getModel("sphere.fbx", shader);
	fbxModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	fbxModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	fbxModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");

	// Create entities
	auto e = Entity::Create("Floor");
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(e);


	// Random cube maze
	const unsigned int mazeStart = 5;
	const unsigned int mazeSize = 20;
	const float wallSize = 1.1f;
	for (unsigned int x = 0; x < mazeSize; x++) {
		for (unsigned int y = 0; y < mazeSize; y++) {
			/*if (Utils::rnd() > 0.5f)
				continue;*/

			e = Entity::Create();
			e->addComponent<ModelComponent>(m_cubeModel.get());
			e->addComponent<TransformComponent>(glm::vec3(x * wallSize + mazeStart, 0.5f, y * wallSize + mazeStart));
			m_scene.addEntity(e);
		}
	}
}

ModelViewerState::~ModelViewerState() {
}

// Process input for the state
bool ModelViewerState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

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
		m_transformTestEntities[2]->getComponent<TransformComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformComponent>());
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_2)) {
		Logger::Log("Removing parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->removeParent();
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

bool ModelViewerState::onEvent(Event& event) {
	SAIL_PROFILE_FUNCTION();
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&ModelViewerState::onResize));

	// Forward events
	m_scene.onEvent(event);

	return true;
}

bool ModelViewerState::onResize(WindowResizeEvent& event) {
	SAIL_PROFILE_FUNCTION();
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool ModelViewerState::update(float dt) {
	SAIL_PROFILE_FUNCTION();

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	return true;
}

// Renders the state
bool ModelViewerState::render(float dt) {
	SAIL_PROFILE_FUNCTION();

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

	return true;
}

bool ModelViewerState::renderImgui(float dt) {
	SAIL_PROFILE_FUNCTION();

	static auto funcSwitchState = [&]() {
		requestStackPop();
		requestStackPush(States::Game);
	};
	static auto modelEnt = Entity::Create();

	// Add to the scene once
	static std::once_flag flag;
	std::call_once(flag, [&] {
		modelEnt->addComponent<TransformComponent>();
		m_scene.addEntity(modelEnt); 
	});
	
	static auto funcNewModel = [&](const std::string& path) {
		Logger::Log("Adding new model to scene: " + path);
		
		auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();
		Model* fbxModel = &m_app->getResourceManager().getModel(path, shader, true);

		// Remove existing model
		if (modelEnt->getComponent<ModelComponent>()) {
			modelEnt->removeComponent<ModelComponent>();
		}
		modelEnt->addComponent<ModelComponent>(fbxModel);

	};

	PhongMaterial* material = nullptr;
	if (auto* modelComp = modelEnt->getComponent<ModelComponent>()) {
		material = modelComp->getModel()->getMesh(0)->getMaterial();
	}

	m_viewerGui.render(dt, funcSwitchState, funcNewModel, material);
	
	return false;
}
