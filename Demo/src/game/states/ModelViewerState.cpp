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
	m_cam.setPosition(glm::vec3(1.6f, 6.7f, 5.4f));
	m_camController.lookAt(glm::vec3(0.f, 4.f, 0.f));
	
	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	// Create/load models
	auto planeMesh = MeshFactory::Plane::Create(glm::vec2(50.f), glm::vec2(30.0f));
	//auto cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f));

	// Create entities
	{
		auto e = m_scene.createEntity("Floor");
		e.addComponent<MeshComponent>(planeMesh);
		e.addComponent<TransformComponent>();
		std::shared_ptr<PBRMaterial> mat = std::make_shared<PBRMaterial>();
		mat->setAlbedoTexture("pbr/pavingStones/albedo.tga");
		mat->setNormalTexture("pbr/pavingStones/normal.tga");
		mat->setMetalnessRoughnessAOTexture("pbr/pavingStones/metalnessRoughnessAO.tga");
		e.addComponent<MaterialComponent>(mat);
	}
	{
		auto e = m_scene.createEntity("Window");
		e.addComponent<MeshComponent>(MeshFactory::Cube::Create({1.f, 1.f, 1.f}));
		e.addComponent<TransformComponent>(glm::vec3(0.f, 4.f, 4.f), glm::vec3(0.f), glm::vec3(1.f, 1.f, 0.05f));
		std::shared_ptr<PBRMaterial> mat = std::make_shared<PBRMaterial>();
		mat->enableTransparency(true);
		mat->setAlbedoTexture("colored_glass_rgba.png");
		e.addComponent<MaterialComponent>(mat);
	}

	// Loads a model and adds it to the scene
	ModelLoader("sponza.fbx", &m_scene).getEntity();

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

		/*for (unsigned int x = 0; x < 10; x++) {
			for (unsigned int y = 0; y < 10; y++) {
				e = m_scene.createEntity("Another point light");
				pl = &e.addComponent<PointLightComponent>();
				pl->setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
				pl->setPosition(glm::vec3(3.f * x, 0.1f, 3.f * y));
			}
		}*/
	}
	// PBR spheres
	{
		auto& sphereMesh = m_app->getResourceManager().loadMesh("sphere.fbx");
		const unsigned int gridSize = 7;
		const float cellSize = 1.3f;
		for (unsigned int x = 0; x < gridSize; x++) {
			for (unsigned int y = 0; y < gridSize; y++) {
				auto e = m_scene.createEntity("Sphere " + std::to_string(x * gridSize + y + 1));
				e.addComponent<MeshComponent>(sphereMesh);
				auto& transform = e.addComponent<TransformComponent>(glm::vec3(x * cellSize - (cellSize * (gridSize - 1.0f) * 0.5f), y * cellSize + 1.0f, 0.f));
				transform.setScale(0.5f);

				std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();
				material->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				// Vary metalness and roughness with cell location
				material->setRoughnessScale(1.f - (x / (float)gridSize));
				material->setMetalnessScale(y / (float)gridSize);
				e.addComponent<MaterialComponent>(material);

			}
		}
	}
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
	m_entitiesGui.render(&m_scene);
	
	return false;
}
