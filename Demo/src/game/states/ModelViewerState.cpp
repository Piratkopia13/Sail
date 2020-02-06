#include "ModelViewerState.h"
#include "imgui.h"
#include "Sail/debug/Instrumentor.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"

#if defined(_SAIL_DX12) && defined(_DEBUG)
#include "API/DX12/DX12API.h"
#endif

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

	auto* phongShader = &m_app->getResourceManager().getShaderSet<PhongMaterialShader>();
	auto* pbrShader = &m_app->getResourceManager().getShaderSet<PBRMaterialShader>(); // Load for the sake of loading (testing)

	// Create/load models
	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(50.f), pbrShader, glm::vec2(30.0f));

	// Create entities
	auto e = Entity::Create("Floor");
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	auto* mat = e->addComponent<MaterialComponent>(Material::PBR);
	mat->get()->asPBR()->setAlbedoTexture("pbr/pavingStones/albedo.tga");
	mat->get()->asPBR()->setNormalTexture("pbr/pavingStones/normal.tga");
	mat->get()->asPBR()->setMetalnessRoughnessAOTexture("pbr/pavingStones/metalnessRoughnessAO.tga");
	m_scene.addEntity(e);


	// PBR spheres
	Model* sphereModel = &m_app->getResourceManager().getModel("sphere.fbx", pbrShader);
	const unsigned int gridSize = 7;
	const float cellSize = 1.3f;
	for (unsigned int x = 0; x < gridSize; x++) {
		for (unsigned int y = 0; y < gridSize; y++) {
			e = Entity::Create();
			auto* model = e->addComponent<ModelComponent>(sphereModel);
			auto* transform = e->addComponent<TransformComponent>(glm::vec3(x * cellSize - (cellSize * (gridSize - 1.0f) * 0.5f), y * cellSize + 1.0f, 0.f));
			transform->setScale(0.5f);

			PBRMaterial* material = e->addComponent<MaterialComponent>(Material::PBR)->get()->asPBR();
			material->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			// Vary metalness and roughness with cell location
			material->setRoughnessScale(1.f - (x / (float)gridSize));
			material->setMetalnessScale(y / (float)gridSize);

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
		m_app->getResourceManager().reloadShader<PhongMaterialShader>();
		m_app->getResourceManager().reloadShader<PBRMaterialShader>();
	}

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

#if defined(_SAIL_DX12) && defined(_DEBUG)
	static int framesToCapture = 3;
	static int frameCounter = 0;
	if (frameCounter < framesToCapture) {
		m_app->getAPI<DX12API>()->beginPIXCapture();
	}
#endif

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

#if defined(_SAIL_DX12) && defined(_DEBUG)
	if (frameCounter < framesToCapture) {
		m_app->getAPI<DX12API>()->endPIXCapture();
		frameCounter++;
	}
#endif

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
		modelEnt->addComponent<MaterialComponent>(Material::PBR);
		m_scene.addEntity(modelEnt); 
	});
	
	static auto funcNewModel = [&](const std::string& path) {
		Logger::Log("Adding new model to scene: " + path);
		
		auto* shader = &m_app->getResourceManager().getShaderSet<PBRMaterialShader>();
		Model* fbxModel = &m_app->getResourceManager().getModel(path, shader, true);

		// Remove existing model
		if (modelEnt->getComponent<ModelComponent>()) {
			modelEnt->removeComponent<ModelComponent>();
		}
		modelEnt->addComponent<ModelComponent>(fbxModel);

	};

	m_viewerGui.render(dt, funcSwitchState, funcNewModel, modelEnt.get());
	
	return false;
}
