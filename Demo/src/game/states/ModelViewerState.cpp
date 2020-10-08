#include "ModelViewerState.h"
#include "imgui.h"

#if defined(_SAIL_DX12) && defined(_DEBUG)
#include "API/DX12/DX12API.h"
#endif

#include "Sail/resources/loaders/ModelLoader.h"

// Command line parsing
#include <shellapi.h>
#include <atlstr.h>

ModelViewerState::ModelViewerState(StateStack& stack)
: State(stack)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
{
	SAIL_PROFILE_FUNCTION();

	// Get the Application instance
	m_app = Application::getInstance();
	
	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 4.7f, 7.4f));
	m_camController.lookAt(glm::vec3(0.f));
	
	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	// Create/load models
	auto planeMesh = MeshFactory::Plane::Create(glm::vec2(50.f), glm::vec2(30.0f));
	//auto cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f));

	// Create entities
	{
		auto e = Entity::Create("Floor");
		e->addComponent<MeshComponent>(planeMesh);
		e->addComponent<TransformComponent>();
		auto mat = e->addComponent<MaterialComponent<PBRMaterial>>();
		mat->get()->setAlbedoTexture("pbr/pavingStones/albedo.tga");
		mat->get()->setNormalTexture("pbr/pavingStones/normal.tga");
		mat->get()->setMetalnessRoughnessAOTexture("pbr/pavingStones/metalnessRoughnessAO.tga");
		m_scene.addEntity(e);
	}
	{
		auto e = Entity::Create("Window");
		e->addComponent<MeshComponent>(MeshFactory::Cube::Create({1.f, 1.f, 1.f}));
		e->addComponent<TransformComponent>(glm::vec3(0.f, 4.f, 4.f), glm::vec3(0.f), glm::vec3(1.f, 1.f, 0.05f));
		auto mat = e->addComponent<MaterialComponent<PBRMaterial>>();
		mat->get()->enableTransparency(true);
		mat->get()->setAlbedoTexture("colored_glass_rgba.png");
		m_scene.addEntity(e);
	}

	ModelLoader testLoader("res/models/sponza.fbx");
	m_scene.addEntity(testLoader.getEntity());
	/*{
		auto e = Entity::Create("Test Model");
		e->addComponent<MeshComponent>(testLoader.getMesh());
		e->addComponent<TransformComponent>();
		auto mat = e->addComponent<MaterialComponent<PBRMaterial>>();
		m_scene.addEntity(e);
	}*/

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

		/*for (unsigned int x = 0; x < 10; x++) {
			for (unsigned int y = 0; y < 10; y++) {
				e = Entity::Create("Another point light");
				pl = e->addComponent<PointLightComponent>();
				pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
				pl->setPosition(glm::vec3(3.f * x, 0.1f, 3.f * y));
				m_scene.addEntity(e);
			}
		}*/
	}
	// PBR spheres
	//{
	//	auto sphereModel = m_app->getResourceManager().getModel("sphere.fbx");
	//	const unsigned int gridSize = 7;
	//	const float cellSize = 1.3f;
	//	for (unsigned int x = 0; x < gridSize; x++) {
	//		for (unsigned int y = 0; y < gridSize; y++) {
	//			auto e = Entity::Create("Sphere " + std::to_string(x * gridSize + y + 1));
	//			auto model = e->addComponent<ModelComponent>(sphereModel);
	//			auto transform = e->addComponent<TransformComponent>(glm::vec3(x * cellSize - (cellSize * (gridSize - 1.0f) * 0.5f), y * cellSize + 1.0f, 0.f));
	//			transform->setScale(0.5f);

	//			PBRMaterial* material = e->addComponent<MaterialComponent<PBRMaterial>>()->get();
	//			material->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	//			// Vary metalness and roughness with cell location
	//			material->setRoughnessScale(1.f - (x / (float)gridSize));
	//			material->setMetalnessScale(y / (float)gridSize);

	//			m_scene.addEntity(e);
	//		}
	//	}
	//}
}

ModelViewerState::~ModelViewerState() {
}

// Process input for the state
bool ModelViewerState::processInput(float dt) {
	SAIL_PROFILE_FUNCTION();

	// Update the camera controller from input devices
	m_camController.update(dt);

	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadAllShaders();
	}

	return true;
}

bool ModelViewerState::update(float dt) {
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
bool ModelViewerState::render(float dt) {
	SAIL_PROFILE_FUNCTION();

#if defined(_SAIL_DX12) && defined(_DEBUG)
	static int framesToCapture = 0;
	static int frameCounter = 0;

	int numArgs;
	LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);
	if (numArgs > 2) {
		std::string arg = std::string(CW2A(args[1]));
		if (arg.find("pixCaptureStartupFrames")) {
			framesToCapture = std::atoi(CW2A(args[2]));
		}
	}
	
	if (frameCounter < framesToCapture) {
		m_app->getAPI<DX12API>()->beginPIXCapture();
	}
#endif

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

	static auto callback = [&](EditorGui::CallbackType type, const std::string& path) {
		switch (type) {
		case EditorGui::CHANGE_STATE:
			requestStackPop();
			requestStackPush(States::Game);

			break;
		case EditorGui::ENVIRONMENT_CHANGED:
			m_scene.getEnvironment()->changeTo(path);

			break;
		default:
			break;
		}
	};

	m_editorGui.render(dt, callback);
	m_entitiesGui.render(m_scene.getEntites());
	
	return false;
}
