#include "GameState.h"
#include "imgui.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/systems/Systems.h"
#include "Sail/ai/states/AttackingState.h"
#include "Sail/graphics/shader/compute/AnimationUpdateComputeShader.h"
#include "Sail/ai/states/FleeingState.h"
#include "Sail/ai/states/SearchingState.h"
#include "Sail/TimeSettings.h"
#include "Sail/utils/GameDataTracker.h"
#include "../SPLASH/src/game/events/NetworkSerializedPackageEvent.h"
#include "Network/NWrapperSingleton.h"

#include <sstream>
#include <iomanip>

// Uncomment to use forward rendering
//#define DISABLE_RT

GameState::GameState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
	, m_profiler(true)
	, m_disableLightComponents(false)
	, m_showcaseProcGen(false) {
	auto& console = Application::getInstance()->getConsole();
	console.addCommand("state <string>", [&](const std::string& param) {
		if (param == "menu") {
			requestStackPop();
			requestStackPush(States::MainMenu);
			m_poppedThisFrame = true;
			console.removeAllCommandsWithIdentifier("GameState");
			return "State change to menu requested";
		} else if (param == "pbr") {
			requestStackPop();
			requestStackPush(States::PBRTest);
			m_poppedThisFrame = true;
			console.removeAllCommandsWithIdentifier("GameState");
			return "State change to pbr requested";
		} else if (param == "perftest") {
			requestStackPop();
			requestStackPush(States::PerformanceTest);
			m_poppedThisFrame = true;
			console.removeAllCommandsWithIdentifier("GameState");
			return "State change to PerformanceTest requested";
		} else {
			return "Invalid state. Available states are \"menu\", \"perftest\" and \"pbr\"";
		}

	}, "GameState");
#ifdef _DEBUG
	console.addCommand("AddCube", [&]() {
		return createCube(m_cam.getPosition());
	}, "GameState");
	console.addCommand("tpmap", [&]() {return teleportToMap(); }, "GameState");
	console.addCommand("AddCube <int> <int> <int>", [&](std::vector<int> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "GameState");
	console.addCommand("AddCube <float> <float> <float>", [&](std::vector<float> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "GameState");
#endif

	// Get the Application instance
	m_app = Application::getInstance();
	m_isSingleplayer = NWrapperSingleton::getInstance().getPlayers().size() == 1;

	//----Octree creation----
	//Wireframe shader
	auto* wireframeShader = &m_app->getResourceManager().getShaderSet<WireframeShader>();

	//Wireframe bounding box model
	Model* boundingBoxModel = &m_app->getResourceManager().getModel("boundingBox.fbx", wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create octree
	m_octree = SAIL_NEW Octree(boundingBoxModel);
	//-----------------------

	// Setting light index
	m_currLightIndex = 0;

	m_componentSystems.movementSystem = ECS::Instance()->createSystem<MovementSystem>();
	
	m_componentSystems.collisionSystem = ECS::Instance()->createSystem<CollisionSystem>();
	m_componentSystems.collisionSystem->provideOctree(m_octree);
	
	m_componentSystems.movementPostCollisionSystem = ECS::Instance()->createSystem<MovementPostCollisionSystem>();

	m_componentSystems.speedLimitSystem = ECS::Instance()->createSystem<SpeedLimitSystem>();
	

	// Create system for animations
	m_componentSystems.animationSystem = ECS::Instance()->createSystem<AnimationSystem>();

	// Create system for updating bounding box
	m_componentSystems.updateBoundingBoxSystem = ECS::Instance()->createSystem<UpdateBoundingBoxSystem>();

	// Create system for handling octree
	m_componentSystems.octreeAddRemoverSystem = ECS::Instance()->createSystem<OctreeAddRemoverSystem>();
	m_componentSystems.octreeAddRemoverSystem->provideOctree(m_octree);
	m_componentSystems.octreeAddRemoverSystem->setCulling(true, &m_cam); // Enable frustum culling

	// Create lifetime system
	m_componentSystems.lifeTimeSystem = ECS::Instance()->createSystem<LifeTimeSystem>();

	// Create entity adder system
	m_componentSystems.entityAdderSystem = ECS::Instance()->getEntityAdderSystem();

	// Create entity removal system
	m_componentSystems.entityRemovalSystem = ECS::Instance()->getEntityRemovalSystem();

	// Create ai system
	m_componentSystems.aiSystem = ECS::Instance()->createSystem<AiSystem>();

	// Create system for the lights
	m_componentSystems.lightSystem = ECS::Instance()->createSystem<LightSystem>();

	// Create system for the candles
	m_componentSystems.candleSystem = ECS::Instance()->createSystem<CandleSystem>();

	// Create system which prepares each new update
	m_componentSystems.prepareUpdateSystem = ECS::Instance()->createSystem<PrepareUpdateSystem>();

	// Create system which handles creation of projectiles
	m_componentSystems.gunSystem = ECS::Instance()->createSystem<GunSystem>();

	// Create system which checks projectile collisions
	m_componentSystems.projectileSystem = ECS::Instance()->createSystem<ProjectileSystem>();

	//create system for level generation
	m_componentSystems.levelGeneratorSystem = ECS::Instance()->createSystem<LevelGeneratorSystem>();

	// Create systems for rendering
	m_componentSystems.beginEndFrameSystem = ECS::Instance()->createSystem<BeginEndFrameSystem>();
	m_componentSystems.boundingboxSubmitSystem = ECS::Instance()->createSystem<BoundingboxSubmitSystem>();
	m_componentSystems.metaballSubmitSystem = ECS::Instance()->createSystem<MetaballSubmitSystem>();
	m_componentSystems.modelSubmitSystem = ECS::Instance()->createSystem<ModelSubmitSystem>();
	m_componentSystems.realTimeModelSubmitSystem = ECS::Instance()->createSystem<RealTimeModelSubmitSystem>();

	// Create system for player input
	m_componentSystems.gameInputSystem = ECS::Instance()->createSystem<GameInputSystem>();
	m_componentSystems.gameInputSystem->initialize(&m_cam);


	// Get the player id's and names from the lobby
	const unsigned char playerID = NWrapperSingleton::getInstance().getMyPlayerID();

	// Create network send and receive systems
	m_componentSystems.networkSenderSystem = ECS::Instance()->createSystem<NetworkSenderSystem>();
	m_componentSystems.networkReceiverSystem = ECS::Instance()->createSystem<NetworkReceiverSystem>();
	m_componentSystems.networkReceiverSystem->initWithPlayerID(playerID);

	// Create system for handling and updating sounds
	m_componentSystems.audioSystem = ECS::Instance()->createSystem<AudioSystem>();


	// Textures needs to be loaded before they can be used
	// TODO: automatically load textures when needed so the following can be removed
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/arenaBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/barrierBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/containerBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/rampBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/candleBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/character1texture.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/Character/CharacterMRAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Character/CharacterNM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Character/CharacterTex.tga");





	// Add a directional light which is used in forward rendering
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));
	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);

	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

#ifdef DISABLE_RT
	auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();
	(*Application::getInstance()->getRenderWrapper()).changeRenderer(1);
	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);
#else
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();
#endif
	m_app->getResourceManager().setDefaultShader(shader);

	// Create/load models
	Model* cubeModel = &m_app->getResourceManager().getModel("cubeWidth1.fbx", shader);
	cubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));
	loadAnimations();

	Model* lightModel = &m_app->getResourceManager().getModel("candleExported.fbx", shader);
	lightModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/candleBasicTexture.tga");

	Model* characterModel = &m_app->getResourceManager().getModel("Character.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Character/CharacterMRAO.tga");
	characterModel->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Character/CharacterTex.tga");
	characterModel->getMesh(0)->getMaterial()->setNormalTexture("pbr/Character/CharacterNM.tga");

	Model* aiModel = &m_app->getResourceManager().getModel("Character.fbx", shader);
	aiModel->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Character/CharacterMRAO.tga");
	aiModel->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Character/CharacterTex.tga");
	aiModel->getMesh(0)->getMaterial()->setNormalTexture("pbr/Character/CharacterNM.tga");

	// Level Creation

	createLevel(shader, boundingBoxModel);

	// Player creation


	int id = static_cast<int>(playerID);
	glm::vec3 spawnLocation;
	for (int i = -1; i < id; i++) {
		spawnLocation = m_componentSystems.levelGeneratorSystem->getSpawnPoint();
	}

	m_player = EntityFactory::CreatePlayer(boundingBoxModel, cubeModel, lightModel, playerID, m_currLightIndex++, spawnLocation).get();

	initAnimations();


	// Inform CandleSystem of the player
	m_componentSystems.candleSystem->setPlayerEntityID(m_player->getID());
	// Bots creation
	createBots(boundingBoxModel, characterModel, cubeModel, lightModel);


#ifdef _DEBUG
	// Candle1 holds all lights you can place in debug...
	m_componentSystems.lightSystem->setDebugLightListEntity("Map_Candle1");
#endif



	// Allocating memory for profiler
	m_virtRAMHistory = SAIL_NEW float[100];
	m_physRAMHistory = SAIL_NEW float[100];
	m_vramUsageHistory = SAIL_NEW float[100];
	m_cpuHistory = SAIL_NEW float[100];
	m_frameTimesHistory = SAIL_NEW float[100];

	for (int i = 0; i < 100; i++) {
		m_virtRAMHistory[i] = 0.f;
		m_physRAMHistory[i] = 0.f;
		m_vramUsageHistory[i] = 0.f;
		m_cpuHistory[i] = 0.f;
		m_frameTimesHistory[i] = 0.f;
	}

	auto nodeSystemCube = ModelFactory::CubeModel::Create(glm::vec3(0.1f), shader);
#ifdef _DEBUG_NODESYSTEM
	m_componentSystems.aiSystem->initNodeSystem(nodeSystemCube.get(), m_octree, wireframeShader);
#else
	m_componentSystems.aiSystem->initNodeSystem(nodeSystemCube.get(), m_octree);
#endif
}

GameState::~GameState() {
	shutDownGameState();

	delete m_virtRAMHistory;
	delete m_physRAMHistory;
	delete m_vramUsageHistory;
	delete m_cpuHistory;
	delete m_frameTimesHistory;
	delete m_octree;
}

// Process input for the state
// NOTE: Done every frame
bool GameState::processInput(float dt) {

#ifdef _DEBUG
	// Add point light at camera pos
	if (Input::WasKeyJustPressed(KeyBinds::addLight)) {
		m_componentSystems.lightSystem->addPointLightToDebugEntity(&m_lights, &m_cam);
	}

#endif

	// Enable bright light and move camera to above procedural generated level
	if (Input::WasKeyJustPressed(KeyBinds::toggleSun)) {
		m_disableLightComponents = !m_disableLightComponents;
		m_showcaseProcGen = m_disableLightComponents;
		if (m_showcaseProcGen) {
			m_lights.getPLs()[0].setPosition(glm::vec3(100.f, 20.f, 100.f));
			m_lights.getPLs()[0].setAttenuation(0.2f, 0.f, 0.f);
		} else {
			m_cam.setPosition(glm::vec3(0.f, 1.f, 0.f));
		}
	}

	// Show boudning boxes
	if (Input::WasKeyJustPressed(KeyBinds::toggleBoundingBoxes)) {
		m_componentSystems.boundingboxSubmitSystem->toggleHitboxes();
	}

	//Test ray intersection
	if (Input::IsKeyPressed(KeyBinds::testRayIntersection)) {
		Octree::RayIntersectionInfo tempInfo;
		m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
		if (tempInfo.info[tempInfo.closestHitIndex].entity) {
			Logger::Log("Ray intersection with " + tempInfo.info[tempInfo.closestHitIndex].entity->getName() + ", " + std::to_string(tempInfo.closestHit) + " meters away");
		}
	}

	//Test frustum culling
	if (Input::IsKeyPressed(KeyBinds::testFrustumCulling)) {
		int nrOfDraws = m_octree->frustumCulledDraw(m_cam);
		Logger::Log("Number of draws " + std::to_string(nrOfDraws));
	}

	// TODO: Move this to a system
	// Toggle ai following the player
	if (Input::WasKeyJustPressed(KeyBinds::toggleAIFollowing)) {
		auto entities = m_componentSystems.aiSystem->getEntities();
		for (int i = 0; i < entities.size(); i++) {
			auto aiComp = entities[i]->getComponent<AiComponent>();
			if (aiComp->entityTarget == nullptr) {

				// Find the candle child entity of player
				Entity* candle = nullptr;
				std::vector<Entity::SPtr> children = m_player->getChildEntities();
				for (auto& child : children) {
					if (child->hasComponent<CandleComponent>()) {
						candle = child.get();
						break;
					}
				}
				aiComp->setTarget(candle);
			} else {
				aiComp->setTarget(nullptr);
			}
		}
	}

	// Set directional light if using forward rendering
	if (Input::IsKeyPressed(KeyBinds::setDirectionalLight)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}
	
	// Reload shaders
	if (Input::WasKeyJustPressed(KeyBinds::reloadShader)) {
		m_app->getResourceManager().reloadShader<AnimationUpdateComputeShader>();
		m_app->getResourceManager().reloadShader<GBufferOutShader>();
	}

	// Pause game
	if (Input::WasKeyJustPressed(KeyBinds::showInGameMenu)) {
		if (m_paused) {
			Input::HideCursor(true);
			m_paused = false;
			requestStackPop();
		} else if (!m_paused && m_isSingleplayer) {
			Input::HideCursor(false);
			m_paused = true;
			requestStackPush(States::Pause);

		}

	}

	if (Input::WasKeyJustPressed(KeyBinds::toggleSphere)) {
		static bool attach = false;
		attach = !attach;
		if (attach) {
			CollisionSpheresComponent* csc = m_player->addComponent<CollisionSpheresComponent>();
			csc->spheres[0].radius = 0.4f;
			csc->spheres[1].radius = csc->spheres[0].radius;
			csc->spheres[0].position = m_player->getComponent<TransformComponent>()->getTranslation() + glm::vec3(0, 1, 0) * (-0.9f + csc->spheres[0].radius);
			csc->spheres[1].position = m_player->getComponent<TransformComponent>()->getTranslation() + glm::vec3(0, 1, 0) * (0.9f - csc->spheres[1].radius);
		} else {
			m_player->removeComponent<CollisionSpheresComponent>();
		}
	}

#ifdef _DEBUG
	// Removes first added pointlight in arena
	if (Input::WasKeyJustPressed(KeyBinds::removeOldestLight)) {
		m_componentSystems.lightSystem->removePointLightFromDebugEntity();
	}
#endif

	return true;
}

bool GameState::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&GameState::onResize));
	EventHandler::dispatch<NetworkSerializedPackageEvent>(event, SAIL_BIND_EVENT(&GameState::onNetworkSerializedPackageEvent));

	EventHandler::dispatch<PlayerCandleDeathEvent>(event, SAIL_BIND_EVENT(&GameState::onPlayerCandleDeath));

	return true;
}

bool GameState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool GameState::onNetworkSerializedPackageEvent(NetworkSerializedPackageEvent& event) {
	m_componentSystems.networkReceiverSystem->pushDataToBuffer(event.getSerializedData());
	return true;
}

bool GameState::onPlayerCandleDeath(PlayerCandleDeathEvent& event) {
	if ( !m_isSingleplayer ) {
		m_player->addComponent<SpectatorComponent>();
		m_player->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.f, 0.f, 0.f);
		m_player->removeComponent<NetworkSenderComponent>();
		m_player->removeComponent<GunComponent>();
		m_player->removeAllChildren();
		// TODO: Remove all the components that can/should be removed
	} else {
		this->requestStackPop();
		this->requestStackPush(States::EndGame);
		m_poppedThisFrame = true;
	}

	return true;
}

bool GameState::update(float dt, float alpha) {
	// UPDATE REAL TIME SYSTEMS
	updatePerFrameComponentSystems(dt, alpha);

	m_lights.updateBufferData();

	return true;
}

bool GameState::fixedUpdate(float dt) {
	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | "
		+ Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1.0f;
	static float change = 0.4f;

	counter += dt * 2.0f;

	updatePerTickComponentSystems(dt);

	return true;
}

// Renders the state
// alpha is a the interpolation value (range [0,1]) between the last two snapshots
bool GameState::render(float dt, float alpha) {
	// Clear back buffer
	m_app->getAPI()->clear({ 0.01f, 0.01f, 0.01f, 1.0f });

	// Draw the scene. Entities with model and trans component will be rendered.
	ECS::Instance();

	m_componentSystems.beginEndFrameSystem->beginFrame(m_cam);
	
	m_componentSystems.modelSubmitSystem->submitAll(alpha);
	m_componentSystems.realTimeModelSubmitSystem->submitAll(alpha);
	m_componentSystems.metaballSubmitSystem->submitAll(alpha);
	m_componentSystems.boundingboxSubmitSystem->submitAll();

	m_componentSystems.beginEndFrameSystem->endFrameAndPresent();

	return true;
}

bool GameState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	renderImguiProfiler(dt);
	renderImGuiRenderSettings(dt);
	renderImGuiLightDebug(dt);
	renderImGuiAnimationSettings(dt);

	return false;
}

bool GameState::prepareStateChange() {
	if (m_poppedThisFrame) {
		// Reset network
		NWrapperSingleton::getInstance().resetNetwork();
	}
	return true;
}

bool GameState::renderImguiProfiler(float dt) {
	bool open = m_profiler.isWindowOpen();
	if (open) {
		if (ImGui::Begin("Profiler", &open)) {

			//Profiler window displaying the current usage
			m_profiler.showWindow(open);
			ImGui::BeginChild("Window", ImVec2(0, 0), false, 0);
			std::string header;

			header = "CPU (" + m_cpuCount + "%%)";
			ImGui::Text(header.c_str());

			header = "Frame time (" + m_ftCount + " seconds)";
			ImGui::Text(header.c_str());

			header = "Virtual RAM (" + m_virtCount + " MB)";
			ImGui::Text(header.c_str());

			header = "Physical RAM (" + m_physCount + " MB)";
			ImGui::Text(header.c_str());

			header = "VRAM (" + m_vramUCount + " MB)";
			ImGui::Text(header.c_str());

			ImGui::Separator();
			//Collapsing headers for graphs over time
			if (ImGui::CollapsingHeader("CPU Graph")) {
				header = "\n\n\n" + m_cpuCount + "(%)";
				ImGui::PlotLines(header.c_str(), m_cpuHistory, 100, 0, "", 0.f, 100.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Frame Times Graph")) {
				header = "\n\n\n" + m_ftCount + "(s)";
				ImGui::PlotLines(header.c_str(), m_frameTimesHistory, 100, 0, "", 0.f, 0.015f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Virtual RAM Graph")) {
				header = "\n\n\n" + m_virtCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_virtRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));

			}
			if (ImGui::CollapsingHeader("Physical RAM Graph")) {
				header = "\n\n\n" + m_physCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_physRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("VRAM Graph")) {
				header = "\n\n\n" + m_vramUCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_vramUsageHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}


			ImGui::EndChild();

			m_profilerTimer += dt;
			//Updating graphs and current usage
			if (m_profilerTimer > 0.2f) {
				m_profilerTimer = 0.f;
				if (m_profilerCounter < 100) {

					m_virtRAMHistory[m_profilerCounter] = (float)m_profiler.virtMemUsage();
					m_physRAMHistory[m_profilerCounter] = (float)m_profiler.workSetUsage();
					m_vramUsageHistory[m_profilerCounter] = (float)m_profiler.vramUsage();
					m_frameTimesHistory[m_profilerCounter] = dt;
					m_cpuHistory[m_profilerCounter++] = (float)m_profiler.processUsage();
					m_virtCount = std::to_string(m_profiler.virtMemUsage());
					m_physCount = std::to_string(m_profiler.workSetUsage());
					m_vramUCount = std::to_string(m_profiler.vramUsage());
					m_cpuCount = std::to_string(m_profiler.processUsage());
					m_ftCount = std::to_string(dt);

				} else {
					// Copying all the history to a new array because ImGui is stupid
					float* tempFloatArr = SAIL_NEW float[100];
					std::copy(m_virtRAMHistory + 1, m_virtRAMHistory + 100, tempFloatArr);
					tempFloatArr[99] = (float)m_profiler.virtMemUsage();
					delete m_virtRAMHistory;
					m_virtRAMHistory = tempFloatArr;
					m_virtCount = std::to_string(m_profiler.virtMemUsage());

					float* tempFloatArr1 = SAIL_NEW float[100];
					std::copy(m_physRAMHistory + 1, m_physRAMHistory + 100, tempFloatArr1);
					tempFloatArr1[99] = (float)m_profiler.workSetUsage();
					delete m_physRAMHistory;
					m_physRAMHistory = tempFloatArr1;
					m_physCount = std::to_string(m_profiler.workSetUsage());

					float* tempFloatArr3 = SAIL_NEW float[100];
					std::copy(m_vramUsageHistory + 1, m_vramUsageHistory + 100, tempFloatArr3);
					tempFloatArr3[99] = (float)m_profiler.vramUsage();
					delete m_vramUsageHistory;
					m_vramUsageHistory = tempFloatArr3;
					m_vramUCount = std::to_string(m_profiler.vramUsage());

					float* tempFloatArr4 = SAIL_NEW float[100];
					std::copy(m_cpuHistory + 1, m_cpuHistory + 100, tempFloatArr4);
					tempFloatArr4[99] = (float)m_profiler.processUsage();
					delete m_cpuHistory;
					m_cpuHistory = tempFloatArr4;
					m_cpuCount = std::to_string(m_profiler.processUsage());

					float* tempFloatArr5 = SAIL_NEW float[100];
					std::copy(m_frameTimesHistory + 1, m_frameTimesHistory + 100, tempFloatArr5);
					tempFloatArr5[99] = dt;
					delete m_frameTimesHistory;
					m_frameTimesHistory = tempFloatArr5;
					m_ftCount = std::to_string(dt);
				}
			}
			ImGui::End();
		} else {
			ImGui::End();
		}
	}

	return false;
}

bool GameState::renderImGuiRenderSettings(float dt) {
	ImGui::Begin("Rendering settings");
	ImGui::Checkbox("Enable post processing",
		&(*Application::getInstance()->getRenderWrapper()).getDoPostProcessing()
	);
	static Entity* pickedEntity = nullptr;
	static float metalness = 1.0f;
	static float roughness = 1.0f;
	static float ao = 1.0f;

	ImGui::Separator();
	if (ImGui::Button("Pick entity")) {
		Octree::RayIntersectionInfo tempInfo;
		m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
		if (tempInfo.closestHitIndex != -1) {
			pickedEntity = tempInfo.info.at(tempInfo.closestHitIndex).entity;
		}
	}

	if (pickedEntity) {
		ImGui::Text("Material properties for %s", pickedEntity->getName().c_str());
		if (auto * model = pickedEntity->getComponent<ModelComponent>()) {
			auto* mat = model->getModel()->getMesh(0)->getMaterial();
			const auto& pbrSettings = mat->getPBRSettings();
			metalness = pbrSettings.metalnessScale;
			roughness = pbrSettings.roughnessScale;
			ao = pbrSettings.aoScale;
			if (ImGui::SliderFloat("Metalness scale", &metalness, 0.f, 1.f)) {
				mat->setMetalnessScale(metalness);
			}
			if (ImGui::SliderFloat("Roughness scale", &roughness, 0.f, 1.f)) {
				mat->setRoughnessScale(roughness);
			}
			if (ImGui::SliderFloat("AO scale", &ao, 0.f, 1.f)) {
				mat->setAOScale(ao);
			}
		}
	}

	ImGui::End();

	return false;
}

bool GameState::renderImGuiLightDebug(float dt) {
	ImGui::Begin("Light debug");
	ImGui::Checkbox("Manual override", &m_disableLightComponents);
	unsigned int i = 0;
	for (auto& pl : m_lights.getPLs()) {
		ImGui::PushID(i);
		std::string label("Point light ");
		label += std::to_string(i);
		if (ImGui::CollapsingHeader(label.c_str())) {

			glm::vec3 color = pl.getColor(); // = 1.0f
			glm::vec3 position = pl.getPosition(); // (12.f, 4.f, 0.f);
			float attConstant = pl.getAttenuation().constant; // 0.312f;
			float attLinear = pl.getAttenuation().linear; // 0.0f;
			float attQuadratic = pl.getAttenuation().quadratic; // 0.0009f;

			ImGui::SliderFloat3("Color##", &color[0], 0.f, 1.0f);
			ImGui::SliderFloat3("Position##", &position[0], -15.f, 15.0f);
			ImGui::SliderFloat("AttConstant##", &attConstant, 0.f, 1.f);
			ImGui::SliderFloat("AttLinear##", &attLinear, 0.f, 1.f);
			ImGui::SliderFloat("AttQuadratic##", &attQuadratic, 0.f, 0.2f);

			pl.setAttenuation(attConstant, attLinear, attQuadratic);
			pl.setColor(color);
			pl.setPosition(position);

		}
		i++;
		ImGui::PopID();
	}
	ImGui::End();
	return true;
}

bool GameState::renderImGuiAnimationSettings(float dt) {
	ImGui::Begin("Animation settings");
	bool interpolate = ECS::Instance()->getSystem<AnimationSystem>()->getInterpolation();
	ImGui::Checkbox("enable animation interpolation", &interpolate);
	ECS::Instance()->getSystem<AnimationSystem>()->setInterpolation(interpolate);
	ImGui::Separator();

	std::vector<Entity*>& e = ECS::Instance()->getSystem<AnimationSystem>()->getEntities();

	if (ImGui::CollapsingHeader("Animated Objects")) {
		for (unsigned int i = 0; i < e.size(); i++) {
			if (ImGui::TreeNode(e[i]->getName().c_str())) {
				AnimationComponent* animationC = e[i]->getComponent<AnimationComponent>();
				ImGui::Text("Animation: %s", animationC->currentAnimation->getName().c_str());
				ImGui::Checkbox("Update on GPU", &animationC->computeUpdate);
				if (ImGui::SliderFloat("Animation Speed", &animationC->animationSpeed, 0.0f, 3.0f)) {

				}
				AnimationStack* stack = animationC->getAnimationStack();
				float w = animationC->animationW;
				ImGui::SliderFloat("weight", &w, 0.0f, 1.0f);
				ImGui::Text("AnimationStack");
				for (unsigned int animationTrack = 0; animationTrack < stack->getAnimationCount(); animationTrack++) {
					float time = -1;
					if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
						time = animationC->animationTime;
					}
					if (animationC->nextAnimation == stack->getAnimation(animationTrack)) {
						if (time > -1) {
							float time2 = animationC->transitions.front().transpiredTime;
							ImGui::SliderFloat(std::string("CurrentTime: " + std::to_string(animationTrack) + "T").c_str(), &time2, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());

						}
						else {
							time = animationC->transitions.front().transpiredTime;

						}
					}
					if (time == -1) {
						time = 0;

					}
					ImGui::SliderFloat(std::string("CurrentTime: "+std::to_string(animationTrack)).c_str(), &time, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());
					if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
						animationC->animationTime = time;
					}
				}

				static float transitionTime = 0.4f;
				static bool transitionWait = false;
				ImGui::Checkbox("transition wait", &transitionWait);
				ImGui::SameLine();
				if (ImGui::SliderFloat("Transition Time", &transitionTime, 0.0f, 1.0f)) {

				}
				for (unsigned int animationIndex = 0; animationIndex < stack->getAnimationCount(); animationIndex++) {

					


					if (ImGui::Button(std::string("Switch to " + stack->getAnimation(animationIndex)->getName()).c_str())) {
						animationC->transitions.emplace(stack->getAnimation(animationIndex), transitionTime, transitionWait);
					}
					ImGui::Separator();
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
	return false;
}

void GameState::shutDownGameState() {

	// Show mouse cursor if hidden
	Input::HideCursor(false);

	ECS::Instance()->stopAllSystems();

	ECS::Instance()->destroyAllEntities();
}

// HERE BE DRAGONS
// Make sure things are updated in the correct order or things will behave strangely
void GameState::updatePerTickComponentSystems(float dt) {
	m_currentlyReadingMask = 0;
	m_currentlyWritingMask = 0;
	m_runningSystemJobs.clear();
	m_runningSystems.clear();

	m_componentSystems.prepareUpdateSystem->update(); // HAS TO BE RUN BEFORE OTHER SYSTEMS WHICH USE TRANSFORM
	
	if (!m_isSingleplayer) {
		// Update entities with info from the network
		m_componentSystems.networkReceiverSystem->update();
		// Send out your entity info to the rest of the players
		m_componentSystems.networkSenderSystem->update();
	}
	
	m_componentSystems.movementSystem->update(dt);
	m_componentSystems.speedLimitSystem->update();
	m_componentSystems.collisionSystem->update(dt);
	m_componentSystems.movementPostCollisionSystem->update(dt);


	// This can probably be used once the respective system developers 
	//	have checked their respective systems for proper component registration
	//runSystem(dt, m_componentSystems.physicSystem); // Needs to be updated before boundingboxes etc.

	// TODO: Investigate this
	// Systems sent to runSystem() need to override the update(float dt) in BaseComponentSystem
	runSystem(dt, m_componentSystems.gunSystem); // TODO: Order?
	runSystem(dt, m_componentSystems.projectileSystem);
	runSystem(dt, m_componentSystems.animationSystem);
	runSystem(dt, m_componentSystems.aiSystem);
	runSystem(dt, m_componentSystems.candleSystem);
	runSystem(dt, m_componentSystems.updateBoundingBoxSystem);
	runSystem(dt, m_componentSystems.octreeAddRemoverSystem);
	runSystem(dt, m_componentSystems.lifeTimeSystem);

	// Wait for all the systems to finish before starting the removal system
	for (auto& fut : m_runningSystemJobs) {
		fut.get();
	}

	// Will probably need to be called last
	m_componentSystems.entityAdderSystem->update();
	m_componentSystems.entityRemovalSystem->update();
}

void GameState::updatePerFrameComponentSystems(float dt, float alpha) {
	// TODO? move to its own thread
	NWrapperSingleton::getInstance().getNetworkWrapper()->checkForPackages();

	// Updates the camera
	m_componentSystems.gameInputSystem->update(dt, alpha);


	// There is an imgui debug toggle to override lights
	if (!m_disableLightComponents) {
		m_lights.clearPointLights();
		//check and update all lights for all entities
		m_componentSystems.lightSystem->updateLights(&m_lights);
	}

	if (m_showcaseProcGen) {
		m_cam.setPosition(glm::vec3(100.f, 100.f, 100.f));
	}
	m_componentSystems.animationSystem->updatePerFrame();
	m_componentSystems.audioSystem->update(m_cam, dt, alpha);
	m_componentSystems.octreeAddRemoverSystem->updatePerFrame(dt);
}

void GameState::runSystem(float dt, BaseComponentSystem* toRun) {
	bool started = false;
	while (!started) {
		// First check if the system can be run
		if (!(m_currentlyReadingMask & toRun->getWriteBitMask()).any() &&
			!(m_currentlyWritingMask & toRun->getReadBitMask()).any() &&
			!(m_currentlyWritingMask & toRun->getWriteBitMask()).any()) {

			m_currentlyWritingMask |= toRun->getWriteBitMask();
			m_currentlyReadingMask |= toRun->getReadBitMask();
			started = true;
			m_runningSystems.push_back(toRun);
			m_runningSystemJobs.push_back(m_app->pushJobToThreadPool([this, dt, toRun](int id) {toRun->update(dt); return toRun; }));

		} else {
			// Then loop through all futures and see if any of them are done
			for (int i = 0; i < m_runningSystemJobs.size(); i++) {
				if (m_runningSystemJobs[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
					auto doneSys = m_runningSystemJobs[i].get();

					m_runningSystemJobs.erase(m_runningSystemJobs.begin() + i);
					i--;

					m_currentlyWritingMask ^= doneSys->getWriteBitMask();
					m_currentlyReadingMask ^= doneSys->getReadBitMask();

					int toRemoveIndex = -1;
					for (int j = 0; j < m_runningSystems.size(); j++) {
						// Currently just compares memory adresses (if they point to the same location they're the same object)
						if (m_runningSystems[j] == doneSys)
							toRemoveIndex = j;
					}

					m_runningSystems.erase(m_runningSystems.begin() + toRemoveIndex);

					// Since multiple systems can read from components concurrently, currently best solution I came up with
					for (auto _sys : m_runningSystems) {
						m_currentlyReadingMask |= _sys->getReadBitMask();
					}
				}
			}
		}
	}
}

void GameState::loadAnimations() {
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();
	m_app->getResourceManager().loadModel("AnimationTest/walkTri.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	//animatedModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

#ifndef _DEBUG
	//m_app->getResourceManager().loadModel("AnimationTest/ScuffedSteve_2.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	//m_app->getResourceManager().loadModel("AnimationTest/BaseMesh_Anim.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	m_app->getResourceManager().loadModel("AnimationTest/DEBUG_BALLBOT.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
#endif
}

void GameState::initAnimations() {
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();

	auto animationEntity2 = ECS::Instance()->createEntity("animatedModel2");
	animationEntity2->addComponent<TransformComponent>();
	animationEntity2->getComponent<TransformComponent>()->translate(-5, 0, 0);
	animationEntity2->getComponent<TransformComponent>()->translate(100.f, 100.f, 100.f);
	animationEntity2->addComponent<ModelComponent>(&m_app->getResourceManager().getModelCopy("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
	animationEntity2->addComponent<AnimationComponent>(&m_app->getResourceManager().getAnimationStack("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<AnimationComponent>()->currentAnimation = animationEntity2->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(0);
	std::string animName = "";
#ifndef _DEBUG
	animName = "AnimationTest/DEBUG_BALLBOT.fbx";
	unsigned int count = m_app->getResourceManager().getAnimationStack(animName).getAnimationCount();
	for (int i = 0; i < 2; i++) {
		auto animationEntity5 = ECS::Instance()->createEntity("DEBUG_BALLBOT-" + std::to_string(i));
		animationEntity5->addComponent<TransformComponent>();
		animationEntity5->getComponent<TransformComponent>()->translate(1.0f+ (i * 2), 1, 0);
		//animationEntity5->getComponent<TransformComponent>()->rotateAroundX(-3.14f*0.5f);
		animationEntity5->getComponent<TransformComponent>()->scale(0.005f);
		animationEntity5->addComponent<ModelComponent>(&m_app->getResourceManager().getModelCopy(animName, shader));
		animationEntity5->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
		animationEntity5->addComponent<AnimationComponent>(&m_app->getResourceManager().getAnimationStack(animName));
		animationEntity5->getComponent<AnimationComponent>()->currentAnimation = animationEntity5->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(i);

	}

	animName = "AnimationTest/BaseMesh_Anim.fbx";
	for (int i = 0; i < 1; i++) {
		auto animationEntity5 = ECS::Instance()->createEntity("BaseMesh_Anim-" + std::to_string(i));
		animationEntity5->addComponent<TransformComponent>();
		animationEntity5->getComponent<TransformComponent>()->translate(-1.0f - (i * 2), 1, 0);
		animationEntity5->getComponent<TransformComponent>()->rotateAroundX(-3.14f * 0.5f);
		animationEntity5->getComponent<TransformComponent>()->scale(0.01f);
		animationEntity5->addComponent<ModelComponent>(&m_app->getResourceManager().getModelCopy(animName, shader));
		animationEntity5->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
		animationEntity5->addComponent<AnimationComponent>(&m_app->getResourceManager().getAnimationStack(animName));
		animationEntity5->getComponent<AnimationComponent>()->currentAnimation = animationEntity5->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(i);

	}
#endif
}

const std::string GameState::teleportToMap() {
	m_player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(30.6f, 0.9f, 40.f));
	return "";
}

const std::string GameState::createCube(const glm::vec3& position) {

	Model* tmpCubeModel = &m_app->getResourceManager().getModel(
		"cubeWidth1.fbx", &m_app->getResourceManager().getShaderSet<GBufferOutShader>());
	tmpCubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));

	Model* tmpbbModel = &m_app->getResourceManager().getModel(
		"boundingBox.fbx", &m_app->getResourceManager().getShaderSet<WireframeShader>());
	tmpCubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));

	auto e = ECS::Instance()->createEntity("new cube");
	e->addComponent<ModelComponent>(tmpCubeModel);

	e->addComponent<TransformComponent>(position);

	e->addComponent<BoundingBoxComponent>(tmpbbModel);

	e->addComponent<CollidableComponent>();

	return std::string("Added Cube at (" +
		std::to_string(position.x) + ":" +
		std::to_string(position.y) + ":" +
		std::to_string(position.z) + ")");
}

void GameState::createTestLevel(Shader* shader, Model* boundingBoxModel) {
	// Load models used for test level
	Model* arenaModel = &m_app->getResourceManager().getModel("arenaBasic.fbx", shader);
	arenaModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/arenaBasicTexture.tga");
	arenaModel->getMesh(0)->getMaterial()->setMetalnessScale(0.570f);
	arenaModel->getMesh(0)->getMaterial()->setRoughnessScale(0.593f);
	arenaModel->getMesh(0)->getMaterial()->setAOScale(0.023f);

	Model* barrierModel = &m_app->getResourceManager().getModel("barrierBasic.fbx", shader);
	barrierModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/barrierBasicTexture.tga");

	Model* containerModel = &m_app->getResourceManager().getModel("containerBasic.fbx", shader);
	containerModel->getMesh(0)->getMaterial()->setMetalnessScale(0.778f);
	containerModel->getMesh(0)->getMaterial()->setRoughnessScale(0.394f);
	containerModel->getMesh(0)->getMaterial()->setAOScale(0.036f);
	containerModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/containerBasicTexture.tga");

	Model* rampModel = &m_app->getResourceManager().getModel("rampBasic.fbx", shader);
	rampModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/rampBasicTexture.tga");
	rampModel->getMesh(0)->getMaterial()->setMetalnessScale(0.0f);
	rampModel->getMesh(0)->getMaterial()->setRoughnessScale(1.0f);
	rampModel->getMesh(0)->getMaterial()->setAOScale(1.0f);

	// Create entities for test level
	
	auto e = EntityFactory::CreateStaticMapObject("Arena", arenaModel, boundingBoxModel, glm::vec3(0.0f , 0.0f, 0.0f));
	e = EntityFactory::CreateStaticMapObject("Map_Barrier1", barrierModel, boundingBoxModel, glm::vec3(-16.15f * 0.3f, 0.f, 3.83f * 0.3f), glm::vec3(0.f, -0.79f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Barrier2", barrierModel, boundingBoxModel, glm::vec3(-4.54f * 0.3f, 0.f, 8.06f * 0.3f));
	e = EntityFactory::CreateStaticMapObject("Map_Barrier3", barrierModel, boundingBoxModel, glm::vec3(8.46f * 0.3f, 0.f, 8.06f * 0.3f));
	e = EntityFactory::CreateStaticMapObject("Map_Container1", containerModel, boundingBoxModel, glm::vec3(6.95f * 0.3f, 0.f, 25.f * 0.3f));
	e = EntityFactory::CreateStaticMapObject("Map_Container2", containerModel, boundingBoxModel, glm::vec3(-25.f * 0.3f, 0.f, 12.43f * 0.3f), glm::vec3(0.f, 1.57f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Container3", containerModel, boundingBoxModel, glm::vec3(-25.f * 0.3f, 2.4f, -7.73f * 0.3f), glm::vec3(0.f, 1.57f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Container4", containerModel, boundingBoxModel, glm::vec3(-19.67f * 0.3f, 0.f, -24.83f * 0.3f), glm::vec3(0.f, 0.79f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Container5", containerModel, boundingBoxModel, glm::vec3(-0.f, 0.f, -14.f * 0.3f));
	e = EntityFactory::CreateStaticMapObject("Map_Container6", containerModel, boundingBoxModel, glm::vec3(24.20f * 0.3f, 0.f, -8.f * 0.3f), glm::vec3(0.f, 1.57f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Container7", containerModel, boundingBoxModel, glm::vec3(24.2f * 0.3f, 2.4f, -22.8f * 0.3f), glm::vec3(0.f, 1.57f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Container8", containerModel, boundingBoxModel, glm::vec3(24.36f * 0.3f, 0.f, -32.41f * 0.3f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp1", rampModel, boundingBoxModel, glm::vec3(5.2f * 0.3f, 0.f, -32.25f * 0.3f), glm::vec3(0.f, 3.14f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp2", rampModel, boundingBoxModel, glm::vec3(15.2f * 0.3f, 2.4f, -32.25f * 0.3f), glm::vec3(0.f, 3.14f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp3", rampModel, boundingBoxModel, glm::vec3(24.f * 0.3f, 2.4f, -5.5f * 0.3f), glm::vec3(0.f, 4.71f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp4", rampModel, boundingBoxModel, glm::vec3(24.f * 0.3f, 0.f, 9.f * 0.3f), glm::vec3(0.f, 4.71f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp5", rampModel, boundingBoxModel, glm::vec3(-16.f * 0.3f, 0.f, 20.f * 0.3f), glm::vec3(0.f, 0.f, 0.f));
	e = EntityFactory::CreateStaticMapObject("Map_Ramp6", rampModel, boundingBoxModel, glm::vec3(-34.f * 0.3f, 0.f, 20.f * 0.3f), glm::vec3(0.f, 3.14f, 0.f));
}

void GameState::createBots(Model* boundingBoxModel, Model* characterModel, Model* projectileModel, Model* lightModel) {
	int botCount = m_app->getStateStorage().getLobbyToGameData()->botCount;

	if (botCount < 0) {
		botCount = 0;
	}

	for (size_t i = 0; i < botCount; i++) {
		auto e = EntityFactory::CreateBot(boundingBoxModel, characterModel, m_componentSystems.levelGeneratorSystem->getSpawnPoint(), lightModel, m_currLightIndex++, m_componentSystems.aiSystem->getNodeSystem());
	}
}

void GameState::createLevel(Shader* shader, Model* boundingBoxModel) {
	std::string tileTex = "sponza/textures/tileTexture1.tga";
	Application::getInstance()->getResourceManager().loadTexture(tileTex);

	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RoomWallMRAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RoomWallNM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RoomWallAlbedo.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RD_MRAo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RD_NM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/RD_Albedo.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CD_MRAo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CD_NM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CD_Albedo.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CW_MRAo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CW_NM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/CW_Albedo.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/F_MRAo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/F_NM.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/Tiles/F_Albedo.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/metalnessRoughnessAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/normal.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/albedo.tga");



	//Load tileset for world
	Model* tileFlat = &m_app->getResourceManager().getModel("Tiles/tileFlat.fbx", shader);
	tileFlat->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	Model* roomWall = &m_app->getResourceManager().getModel("Tiles/RoomWall.fbx", shader);
	roomWall->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Tiles/RoomWallMRAO.tga");
	roomWall->getMesh(0)->getMaterial()->setNormalTexture("pbr/Tiles/RoomWallNM.tga");
	roomWall->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Tiles/RoomWallAlbedo.tga");

	Model* tileDoor = &m_app->getResourceManager().getModel("Tiles/tileDoor.fbx", shader);
	tileDoor->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	Model* roomDoor = &m_app->getResourceManager().getModel("Tiles/RoomDoor.fbx", shader);
	//roomDoor->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);
	roomDoor->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Tiles/RD_MRAo.tga");
	roomDoor->getMesh(0)->getMaterial()->setNormalTexture("pbr/Tiles/RD_NM.tga");
	roomDoor->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Tiles/RD_Albedo.tga");


	Model* corridorDoor = &m_app->getResourceManager().getModel("Tiles/CorridorDoor.fbx", shader);
	corridorDoor->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Tiles/CD_MRAo.tga");
	corridorDoor->getMesh(0)->getMaterial()->setNormalTexture("pbr/Tiles/CD_NM.tga");
	corridorDoor->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Tiles/CD_Albedo.tga");

	Model* corridorWall = &m_app->getResourceManager().getModel("Tiles/CorridorWall.fbx", shader);
	//corridorWall->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);
	corridorWall->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Tiles/CW_MRAo.tga");
	corridorWall->getMesh(0)->getMaterial()->setNormalTexture("pbr/Tiles/CW_NM.tga");
	corridorWall->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Tiles/CW_Albedo.tga");

	Model* ceiling = &m_app->getResourceManager().getModel("Tiles/RoomCeiling.fbx", shader);
	ceiling->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	Model* floor = &m_app->getResourceManager().getModel("Tiles/RoomFloor.fbx", shader);
	//floor->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);
	floor->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/Tiles/F_MRAo.tga");
	floor->getMesh(0)->getMaterial()->setNormalTexture("pbr/Tiles/F_NM.tga");
	floor->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/Tiles/F_Albedo.tga");


	std::vector<Model*> tileModels;
	tileModels.resize(TileModel::NUMBOFMODELS);
	tileModels[TileModel::ROOM_FLOOR] = floor;
	tileModels[TileModel::ROOM_WALL] = roomWall;
	tileModels[TileModel::ROOM_DOOR] = roomDoor;

	tileModels[TileModel::CORRIDOR_FLOOR] = floor;
	tileModels[TileModel::CORRIDOR_WALL] = corridorWall;
	tileModels[TileModel::CORRIDOR_DOOR] = corridorDoor;



	// Create the level generator system and put it into the datatype.
	auto map = ECS::Instance()->createEntity("Map");
	map->addComponent<MapComponent>();
	ECS::Instance()->addAllQueuedEntities();
	m_componentSystems.levelGeneratorSystem->generateMap();
	m_componentSystems.levelGeneratorSystem->createWorld(tileModels, boundingBoxModel);
}

