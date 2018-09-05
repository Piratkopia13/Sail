#include "GameState.h"
#include "../objects/Block.h"

#include <Effects.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;


GameState::GameState(StateStack& stack)
: State(stack)
, m_cam(30.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam, 90.f, -90.f, 0.f)
, m_flyCam(true)
, m_fpsText(nullptr)
, m_debugCamText(nullptr)
{

	// Get the Application instance
	m_app = Application::getInstance();

	// Load textures
	m_app->getResourceManager().loadDXTexture("board.tga");

	//m_scene = std::make_unique<Scene>(AABB(Vector3(-100.f, -100.f, -100.f), Vector3(100.f, 100.f, 100.f)));

	// Set up camera with controllers
	m_cam.setPosition(Vector3(0.f, 3.f, -1.0f));
	
	// Set up the scene
	//m_scene.addSkybox(L"skybox_space_512.dds");
	// Add a directional light
	Vector3 color(1.0f, 1.0f, 1.0f);
 	Vector3 direction(0.0488522f, -0.943805f, 0.326874f);
	direction.Normalize();
	m_lights.setDirectionalLight(DirectionalLight(color, direction));


	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

	auto* shader = &m_app->getResourceManager().getShaderSet<DeferredGeometryShader>();
	//auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();

	m_cubeModel = ModelFactory::CubeModel::Create(Vector3(.3f), shader);
	m_cubeModel->getMesh(0)->getMaterial()->setDiffuseTexture("missing.tga");
	//m_cubeModel = ModelFactory::PlaneModel::Create(Vector2(.5f), shader);

	m_planeModel = ModelFactory::PlaneModel::Create(Vector2(1.f, 0.5f), shader);
	m_planeModel->getMesh(0)->getMaterial()->setDiffuseTexture("board.tga");

	m_scene.setLightSetup(&m_lights);

	auto e = Entity::Create();
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setRotations(Vector3(0.f, 1.f, 0.f));
	e->addComponent<TransformComponent>()->getTransform().setScale(0.3f);
	m_turnIndicatorEntity = m_scene.addEntity(MOVE(e));


	// Kahala board

	e = Entity::Create();
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>()->getTransform().setTranslation(Vector3(0.f, 0.f, -1.f));
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


	// TEST
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Application::getInstance()->getAPI()->getDeviceContext());
	m_states = std::make_unique<CommonStates>(m_app->getAPI()->getDevice());


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
		Logger::Log(Utils::vec3ToStr(m_lights.getDL().getDirection()));
	}

	// Update the camera controller from input devices
	if (m_flyCam)
		m_camController.update(dt);

	// Reload shaders
	if (kbTracker.pressed.R) {
		m_app->getResourceManager().reloadShader<DeferredGeometryShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
	}


	// Kalaha game controls
	if (kbTracker.pressed.D1)
		m_kalahaGame.play(0);
	if (kbTracker.pressed.D2)
		m_kalahaGame.play(1);
	if (kbTracker.pressed.D3)
		m_kalahaGame.play(2);
	if (kbTracker.pressed.D4)
		m_kalahaGame.play(3);
	if (kbTracker.pressed.D5)
		m_kalahaGame.play(4);
	if (kbTracker.pressed.D6)
		m_kalahaGame.play(5);

	if (kbTracker.pressed.D0)
		m_kalahaGame.reset();


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
	////m_scene->resize(width, height);
	return true;
}

bool GameState::update(float dt) {

	std::wstring fps = std::to_wstring(m_app->getFPS());
	// Update HUD texts
	if (m_fpsText)
		m_fpsText->setText(L"FPS: " + fps);

	auto& camPos = m_cam.getPosition();
	if (m_debugCamText)
		m_debugCamText->setText(L"Camera @ " + Utils::vec3ToWStr(camPos) + L"\n" + 
			L"GPU memory usage: " + std::to_wstring(m_app->getAPI()->getMemoryUsage()) + L"/" + std::to_wstring(m_app->getAPI()->getMemoryBudget()) + L"mb");

	m_app->getWindow()->setWindowTitle(L"Sail | Game Engine Demo | FPS: " + fps);

	// Place turn indicator model at the correct corner
	int turn = m_kalahaGame.getCurrentTurn();
	if (turn == 0)
		m_turnIndicatorEntity->getComponent<TransformComponent>()->getTransform().setTranslation(Vector3(0.8f, 0.1f, -1.4f));
	else
		m_turnIndicatorEntity->getComponent<TransformComponent>()->getTransform().setTranslation(Vector3(-0.8f, 0.1f, -0.6f));

	if (m_kalahaGame.isGameOver()) {
		Logger::Log("Game is over! - press 0 to reset");
	}

	return true;
}

// Renders the state
bool GameState::render(float dt) {

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);


	// Kalaha 3D text rendering for the seed counters

	Vector3 textPosition(0, 0, 0);

	Matrix m_world = DirectX::XMMatrixScaling(1, -1, 1) * XMMatrixTranslationFromVector(textPosition) * XMMatrixRotationX(XM_PIDIV2);
	Matrix m_view = m_cam.getViewMatrix();
	Matrix m_proj = m_cam.getProjMatrix();
	Matrix WVP = m_world * m_view * m_proj;

	auto samplerState = m_states->PointWrap();
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, nullptr, samplerState, nullptr, nullptr, nullptr, WVP);
	m_spriteBatch->SetRotation(DXGI_MODE_ROTATION_UNSPECIFIED); // This needs to be here for magic reasons

	// Get counters from kalaha and draw them in the correct houses/stores

	for (int houseNumber = 0; houseNumber < 6; houseNumber++) {
		std::wstring message = std::to_wstring(m_kalahaGame.getHouseCount(0, houseNumber));
		m_font.get()->DrawString(m_spriteBatch.get(), message.c_str(), Vector2(houseNumber * 0.21f - 0.6f, 1.21f), DirectX::Colors::White, 0.f, Vector2::Zero, Vector2(0.005f));
	}
	for (int houseNumber = 0; houseNumber < 6; houseNumber++) {
		std::wstring message = std::to_wstring(m_kalahaGame.getHouseCount(1, houseNumber));
		m_font.get()->DrawString(m_spriteBatch.get(), message.c_str(), Vector2( (5 - houseNumber) * 0.21f - 0.6f, 0.70f), DirectX::Colors::White, 0.f, Vector2::Zero, Vector2(0.005f));
	}

	std::wstring message = std::to_wstring(m_kalahaGame.getStoreCount(1));
	m_font.get()->DrawString(m_spriteBatch.get(), message.c_str(), Vector2(-0.8f, 0.9f), DirectX::Colors::White, 0.f, Vector2::Zero, Vector2(0.005f));

	message = std::to_wstring(m_kalahaGame.getStoreCount(0));
	m_font.get()->DrawString(m_spriteBatch.get(), message.c_str(), Vector2(0.7f, 0.9f), DirectX::Colors::White, 0.f, Vector2::Zero, Vector2(0.005f));


	m_spriteBatch->End();

	// Re-enable the depth buffer and rasterizer state after 2D rendering
	m_app->getAPI()->setDepthMask(GraphicsAPI::NO_MASK);

	// END OF 3D text rendering


	// Draw HUD
	//m_scene->drawHUD();

	return true;
}
