#include "GameState.h"
#include "../objects/Block.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


GameState::GameState(StateStack& stack)
: State(stack)
, m_cam(30.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
, m_flyCam(true)
, m_fpsText(nullptr)
, m_debugCamText(nullptr)
{

	// Get the Application instance
	m_app = Application::getInstance();
	//m_scene = std::make_unique<Scene>(AABB(Vector3(-100.f, -100.f, -100.f), Vector3(100.f, 100.f, 100.f)));

	// Set up camera with controllers
	m_cam.setPosition(Vector3(1.5f, 1.f, -3.0f));
	
	// Set up the scene
	//m_scene->addSkybox(L"skybox_space_512.dds");
	// Add a directional light
	Vector3 color(1.0f, 1.0f, 1.0f);
 	Vector3 direction(0.4f, -0.6f, 1.0f);
	direction.Normalize();
	m_lights.setDirectionalLight(DirectionalLight(color, direction));


	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	m_cubeModel = ModelFactory::CubeModel::Create(Vector3(.5f), &m_app->getResourceManager().getShaderSet<MaterialShader>());
	m_cubeModel->getMesh(0)->getMaterial()->setDiffuseTexture("missing.tga");
	//m_cubeModel = ModelFactory::PlaneModel::Create(Vector2(.5f), &m_app->getResourceManager().getShaderSet<MaterialShader>());

	m_scene.setLightSetup(&m_lights);

	auto e = Entity::Create();
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setRotations(Vector3(0.f, 0.f, 1.07f));
	m_scene.addEntity(MOVE(e));

	e = Entity::Create();
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setTranslation(Vector3(0.f, 1.f, 0.f));
	m_scene.addEntity(MOVE(e));

	Model* fbxModel = &m_app->getResourceManager().getModel("sponza.fbx", &m_app->getResourceManager().getShaderSet<MaterialShader>());

	e = Entity::Create();
	e->addComponent<ModelComponent>(fbxModel);
	e->addComponent<TransformComponent>()->getTransform().setTranslation(Vector3(0.f, 3.f, 0.f));
	m_scene.addEntity(MOVE(e));

	e = Entity::Create();
	auto* textComp = e->addComponent<TextComponent>();
	m_fpsText = textComp->addText(Text::Create(L"FPSText"));
	m_debugCamText = textComp->addText(Text::Create(L"CamText"));
	m_scene.addEntity(MOVE(e));

	// Set up HUD texts
	if (m_debugCamText)
		m_debugCamText->setPosition(Vector2(0.f, 20.f));
	// Add texts to the scene
	//m_scene->addText(&m_fpsText);
#ifdef _DEBUG
	//m_scene->addText(&m_debugCamText);
#endif

}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {

	auto& kbTracker = m_app->getInput().getKbStateTracker();
	auto& kbState = m_app->getInput().getKeyboardState();

#ifdef _DEBUG
	// Toggle camera controller on 'F' key or 'Y' btn
	if (kbTracker.pressed.F)
		m_flyCam = !m_flyCam;
	// Add point light at camera pos
	if (kbTracker.pressed.E) {
		PointLight pl;
		pl.setColor(Vector3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(m_cam.getPosition());
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		m_lights.addPointLight(pl);
	}
#endif

	if (kbState.G) {
		Vector3 color(1.0f, 1.0f, 1.0f);;
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Update the camera controller from input devices
	if (m_flyCam)
		m_camController.update(dt);

	// Reload shaders
	if (kbState.R) {
		m_app->getResourceManager().reloadShader<MaterialShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
	}


	return true;
}

void GameState::onEvent(Event& event) {
	Logger::Log("Recieved event: " + std::to_string(event.getType()));

	EventHandler::dispatch<WindowResizeEvent>(event, FUNC(&GameState::onResize));
}

bool GameState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	////m_scene->resize(width, height);
	return true;
}

// Updates the state
bool GameState::update(float dt) {

	std::wstring fps = std::to_wstring(m_app->getFPS());
	// Update HUD texts
	if (m_fpsText)
		m_fpsText->setText(L"FPS: " + fps);

	auto& camPos = m_cam.getPosition();
	if (m_debugCamText)
		m_debugCamText->setText(L"Camera @ " + Utils::vec3ToWStr(camPos));

	m_app->getWindow()->setWindowTitle(L"Sail | Game Engine Demo | FPS: " + fps);

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
