#include "PerformanceTestState.h"
#include "imgui.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/systems/Systems.h"
#include "Sail/TimeSettings.h"
#include "Sail/utils/GameDataTracker.h"
#include "Network/NWrapperSingleton.h"

#include <sstream>
#include <iomanip>

// Uncomment to use forward rendering
//#define DISABLE_RT

PerformanceTestState::PerformanceTestState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
	, m_camController(&m_cam)
	, m_disableLightComponents(false)
	, m_showcaseProcGen(false) {
	auto& console = Application::getInstance()->getConsole();
	console.addCommand("state <string>", [&](const std::string& param) {
		if (param == "menu") {
			requestStackPop();
			requestStackPush(States::MainMenu);
			console.removeAllCommandsWithIdentifier("PerfTestState");
			return "State change to menu requested";
		} else {
			return "Invalid state. Available states are \"menu\"";
		}
		}, "PerfTestState");

#ifndef _PERFORMANCE_TEST
	Logger::Error("Don't run this state in any other configuration than PerformanceTest");
#endif

	// Get the Application instance
	m_app = Application::getInstance();
	
	// Create systems for rendering
	m_componentSystems.beginEndFrameSystem = ECS::Instance()->createSystem<BeginEndFrameSystem>();
	m_componentSystems.boundingboxSubmitSystem = ECS::Instance()->createSystem<BoundingboxSubmitSystem>();
	m_componentSystems.metaballSubmitSystem = ECS::Instance()->createSystem<MetaballSubmitSystem>();
	m_componentSystems.modelSubmitSystem = ECS::Instance()->createSystem<ModelSubmitSystem>();
	m_componentSystems.realTimeModelSubmitSystem = ECS::Instance()->createSystem<RealTimeModelSubmitSystem>();

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
	( *Application::getInstance()->getRenderWrapper() ).changeRenderer(1);
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

	Model* characterModel = &m_app->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setMetalnessScale(0.0f);
	characterModel->getMesh(0)->getMaterial()->setRoughnessScale(0.217f);
	characterModel->getMesh(0)->getMaterial()->setAOScale(0.0f);
	characterModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/character1texture.tga");

	Model* aiModel = &m_app->getResourceManager().getModel("cylinderRadii0_7.fbx", shader);
	aiModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/character1texture.tga");

	//initAnimations();
	// Level Creation
	createLevel(shader, boundingBoxModel);

	// Bots creation
	createBots(boundingBoxModel, characterModel, cubeModel, lightModel);

	// Populate the performance test scene
	populateScene(characterModel, lightModel, boundingBoxModel, cubeModel, shader);




	// Allocating memory for profiler
	m_virtRAMHistory = SAIL_NEW float[100];
	m_physRAMHistory = SAIL_NEW float[100];
	m_vramUsageHistory = SAIL_NEW float[100];
	m_cpuHistory = SAIL_NEW float[100];
	m_frameTimesHistory = SAIL_NEW float[100];

	for ( int i = 0; i < 100; i++ ) {
		m_virtRAMHistory[i] = 0.f;
		m_physRAMHistory[i] = 0.f;
		m_vramUsageHistory[i] = 0.f;
		m_cpuHistory[i] = 0.f;
		m_frameTimesHistory[i] = 0.f;
	}

	auto nodeSystemCube = ModelFactory::CubeModel::Create(glm::vec3(0.1f), shader);
	m_componentSystems.aiSystem->initNodeSystem(nodeSystemCube.get(), m_octree);

	m_camController.setCameraPosition(glm::vec3(105.83f, 1.7028f, 99.2561f));
	//m_cam.setDirection(glm::normalize(glm::vec3(-0.769527f, -0.202786f, 0.605563f)));
	m_camController.setDirection(glm::normalize(glm::vec3(-0.715708f, 0.0819399f, 0.693576f)));
}

PerformanceTestState::~PerformanceTestState() {
	shutDownPerformanceTestState();

	delete m_virtRAMHistory;
	delete m_physRAMHistory;
	delete m_vramUsageHistory;
	delete m_cpuHistory;
	delete m_frameTimesHistory;
	delete m_octree;
}

// Process input for the state
// NOTE: Done every frame
bool PerformanceTestState::processInput(float dt) {

	// Show boudning boxes
	if ( Input::WasKeyJustPressed(KeyBinds::toggleBoundingBoxes) ) {
		m_componentSystems.boundingboxSubmitSystem->toggleHitboxes();
	}

	//Test ray intersection
	if ( Input::IsKeyPressed(KeyBinds::testRayIntersection) ) {
		Octree::RayIntersectionInfo tempInfo;
		m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
		if ( tempInfo.info[tempInfo.closestHitIndex].entity ) {
			Logger::Log("Ray intersection with " + tempInfo.info[tempInfo.closestHitIndex].entity->getName() + ", " + std::to_string(tempInfo.closestHit) + " meters away");
		}
	}

	//Test frustum culling
	if ( Input::IsKeyPressed(KeyBinds::testFrustumCulling) ) {
		int nrOfDraws = m_octree->frustumCulledDraw(m_cam);
		Logger::Log("Number of draws " + std::to_string(nrOfDraws));
	}

	// TODO: Move this to a system
	// Toggle ai following the player
	if ( Input::WasKeyJustPressed(KeyBinds::toggleAIFollowing) ) {
		/*auto entities = m_componentSystems.aiSystem->getEntities();
		for ( int i = 0; i < entities.size(); i++ ) {
			auto aiComp = entities[i]->getComponent<AiComponent>();
			if ( aiComp->entityTarget == nullptr ) {

				// Find the candle child entity of player
				Entity* candle = nullptr;
				std::vector<Entity::SPtr> children = m_player->getChildEntities();
				for ( auto& child : children ) {
					if ( child->hasComponent<CandleComponent>() ) {
						candle = child.get();
						break;
					}
				}
				aiComp->setTarget(candle);
			} else {
				aiComp->setTarget(nullptr);
			}
		}*/
	}

	// Set directional light if using forward rendering
	if ( Input::IsKeyPressed(KeyBinds::setDirectionalLight) ) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Reload shaders
	if ( Input::WasKeyJustPressed(KeyBinds::reloadShader) ) {
		m_app->getResourceManager().reloadShader<GBufferOutShader>();
	}

	// Pause game
	if ( Input::WasKeyJustPressed(KeyBinds::showInGameMenu) ) {
		if ( m_paused ) {
			Input::HideCursor(true);
			m_paused = false;
			requestStackPop();
		} else if ( !m_paused && m_isSingleplayer ) {
			Input::HideCursor(false);
			m_paused = true;
			requestStackPush(States::Pause);

		}

	}

	if ( Input::WasKeyJustPressed(KeyBinds::toggleSphere) ) {
		/*static bool attach = false;
		attach = !attach;
		if ( attach ) {
			CollisionSpheresComponent* csc = m_player->addComponent<CollisionSpheresComponent>();
			csc->spheres[0].radius = 0.4f;
			csc->spheres[1].radius = csc->spheres[0].radius;
			csc->spheres[0].position = m_player->getComponent<TransformComponent>()->getTranslation() + glm::vec3(0, 1, 0) * ( -0.9f + csc->spheres[0].radius );
			csc->spheres[1].position = m_player->getComponent<TransformComponent>()->getTranslation() + glm::vec3(0, 1, 0) * ( 0.9f - csc->spheres[1].radius );
		} else {
			m_player->removeComponent<CollisionSpheresComponent>();
		}*/
	}

	m_camController.update(dt);


	return true;
}

bool PerformanceTestState::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&PerformanceTestState::onResize));

	return true;
}

bool PerformanceTestState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool PerformanceTestState::update(float dt, float alpha) {
	// UPDATE REAL TIME SYSTEMS
	updatePerFrameComponentSystems(dt, alpha);

	m_lights.updateBufferData();

	return true;
}

bool PerformanceTestState::fixedUpdate(float dt) {
	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | "
									   + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1.0f;
	static float change = 0.4f;

	counter += dt * 2.0f;

	/* here we shoot the guns */
	for ( auto e : m_performanceEntities ) {
		auto pos = m_camController.getCameraPosition();
		auto ePos = e->getComponent<TransformComponent>()->getTranslation();
		ePos.y = ePos.y + 5.f;
		auto dir = ePos - pos;
		auto dirNorm = glm::normalize(dir);
		e->getComponent<GunComponent>()->setFiring(pos + dirNorm * 3.f, glm::vec3(0.f, -1.f, 0.f));
	}

	updatePerTickComponentSystems(dt);

	return true;
}

// Renders the state
// alpha is a the interpolation value (range [0,1]) between the last two snapshots
bool PerformanceTestState::render(float dt, float alpha) {
	// Clear back buffer
	m_app->getAPI()->clear({ 0.01f, 0.01f, 0.01f, 1.0f });

	// Draw the scene. Entities with model and trans component will be rendered.
	m_componentSystems.beginEndFrameSystem->beginFrame(m_cam);

	m_componentSystems.modelSubmitSystem->submitAll(alpha);
	m_componentSystems.realTimeModelSubmitSystem->submitAll(alpha);
	m_componentSystems.metaballSubmitSystem->submitAll(alpha);
	m_componentSystems.boundingboxSubmitSystem->submitAll();

	m_componentSystems.beginEndFrameSystem->endFrameAndPresent();

	return true;
}

bool PerformanceTestState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	renderImguiProfiler(dt);
	renderImGuiRenderSettings(dt);
	renderImGuiLightDebug(dt);
	renderImGuiGameValues(dt);

	return false;
}

bool PerformanceTestState::prepareStateChange() {
	if ( m_poppedThisFrame ) {
		// Reset network
		//NWrapperSingleton::getInstance().resetNetwork();
	}
	return true;
}

bool PerformanceTestState::renderImguiProfiler(float dt) {
	bool open = m_profiler.isWindowOpen();
	if ( open ) {
		if ( ImGui::Begin("Profiler", &open) ) {
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
			if ( ImGui::CollapsingHeader("CPU Graph") ) {
				header = "\n\n\n" + m_cpuCount + "(%)";
				ImGui::PlotLines(header.c_str(), m_cpuHistory, 100, 0, "", 0.f, 100.f, ImVec2(0, 100));
			}
			if ( ImGui::CollapsingHeader("Frame Times Graph") ) {
				header = "\n\n\n" + m_ftCount + "(s)";
				ImGui::PlotLines(header.c_str(), m_frameTimesHistory, 100, 0, "", 0.f, 0.015f, ImVec2(0, 100));
			}
			if ( ImGui::CollapsingHeader("Virtual RAM Graph") ) {
				header = "\n\n\n" + m_virtCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_virtRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));

			}
			if ( ImGui::CollapsingHeader("Physical RAM Graph") ) {
				header = "\n\n\n" + m_physCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_physRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
			if ( ImGui::CollapsingHeader("VRAM Graph") ) {
				header = "\n\n\n" + m_vramUCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_vramUsageHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}


			ImGui::EndChild();

			m_profilerTimer += dt;
			//Updating graphs and current usage
			if ( m_profilerTimer > 0.2f ) {
				m_profilerTimer = 0.f;
				if ( m_profilerCounter < 100 ) {

					m_virtRAMHistory[m_profilerCounter] = ( float ) m_profiler.virtMemUsage();
					m_physRAMHistory[m_profilerCounter] = ( float ) m_profiler.workSetUsage();
					m_vramUsageHistory[m_profilerCounter] = ( float ) m_profiler.vramUsage();
					m_frameTimesHistory[m_profilerCounter] = dt;
					m_cpuHistory[m_profilerCounter++] = ( float ) m_profiler.processUsage();
					m_virtCount = std::to_string(m_profiler.virtMemUsage());
					m_physCount = std::to_string(m_profiler.workSetUsage());
					m_vramUCount = std::to_string(m_profiler.vramUsage());
					m_cpuCount = std::to_string(m_profiler.processUsage());
					m_ftCount = std::to_string(dt);

				} else {
					// Copying all the history to a new array because ImGui is stupid
					float* tempFloatArr = SAIL_NEW float[100];
					std::copy(m_virtRAMHistory + 1, m_virtRAMHistory + 100, tempFloatArr);
					tempFloatArr[99] = ( float ) m_profiler.virtMemUsage();
					delete m_virtRAMHistory;
					m_virtRAMHistory = tempFloatArr;
					m_virtCount = std::to_string(m_profiler.virtMemUsage());

					float* tempFloatArr1 = SAIL_NEW float[100];
					std::copy(m_physRAMHistory + 1, m_physRAMHistory + 100, tempFloatArr1);
					tempFloatArr1[99] = ( float ) m_profiler.workSetUsage();
					delete m_physRAMHistory;
					m_physRAMHistory = tempFloatArr1;
					m_physCount = std::to_string(m_profiler.workSetUsage());

					float* tempFloatArr3 = SAIL_NEW float[100];
					std::copy(m_vramUsageHistory + 1, m_vramUsageHistory + 100, tempFloatArr3);
					tempFloatArr3[99] = ( float ) m_profiler.vramUsage();
					delete m_vramUsageHistory;
					m_vramUsageHistory = tempFloatArr3;
					m_vramUCount = std::to_string(m_profiler.vramUsage());

					float* tempFloatArr4 = SAIL_NEW float[100];
					std::copy(m_cpuHistory + 1, m_cpuHistory + 100, tempFloatArr4);
					tempFloatArr4[99] = ( float ) m_profiler.processUsage();
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

bool PerformanceTestState::renderImGuiRenderSettings(float dt) {
	ImGui::Begin("Rendering settings");
	ImGui::Checkbox("Enable post processing",
					&( *Application::getInstance()->getRenderWrapper() ).getDoPostProcessing()
	);
	bool interpolate = ECS::Instance()->getSystem<AnimationSystem>()->getInterpolation();
	ImGui::Checkbox("enable animation interpolation", &interpolate);
	ECS::Instance()->getSystem<AnimationSystem>()->setInterpolation(interpolate);
	static Entity* pickedEntity = nullptr;
	static float metalness = 1.0f;
	static float roughness = 1.0f;
	static float ao = 1.0f;

	ImGui::Separator();
	if ( ImGui::Button("Pick entity") ) {
		Octree::RayIntersectionInfo tempInfo;
		m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
		if ( tempInfo.closestHitIndex != -1 ) {
			pickedEntity = tempInfo.info.at(tempInfo.closestHitIndex).entity;
		}
	}

	if ( pickedEntity ) {
		ImGui::Text("Material properties for %s", pickedEntity->getName().c_str());
		if ( auto * model = pickedEntity->getComponent<ModelComponent>() ) {
			auto* mat = model->getModel()->getMesh(0)->getMaterial();
			const auto& pbrSettings = mat->getPBRSettings();
			metalness = pbrSettings.metalnessScale;
			roughness = pbrSettings.roughnessScale;
			ao = pbrSettings.aoScale;
			if ( ImGui::SliderFloat("Metalness scale", &metalness, 0.f, 1.f) ) {
				mat->setMetalnessScale(metalness);
			}
			if ( ImGui::SliderFloat("Roughness scale", &roughness, 0.f, 1.f) ) {
				mat->setRoughnessScale(roughness);
			}
			if ( ImGui::SliderFloat("AO scale", &ao, 0.f, 1.f) ) {
				mat->setAOScale(ao);
			}
		}
	}

	ImGui::End();

	return false;
}

bool PerformanceTestState::renderImGuiLightDebug(float dt) {
	ImGui::Begin("Light debug");
	ImGui::Checkbox("Manual override", &m_disableLightComponents);
	unsigned int i = 0;
	for ( auto& pl : m_lights.getPLs() ) {
		ImGui::PushID(i);
		std::string label("Point light ");
		label += std::to_string(i);
		if ( ImGui::CollapsingHeader(label.c_str()) ) {

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

bool PerformanceTestState::renderImGuiGameValues(float dt) {
	ImGui::Begin("Game Values");
	std::string header;

	header = "Player position: " + Utils::toStr(m_camController.getCameraPosition());
	ImGui::Text(header.c_str());

	header = "Player dir: " + Utils::toStr(m_camController.getCameraDirection());
	ImGui::Text(header.c_str());

	ImGui::Separator();
	//Collapsing headers for graphs over time
	if ( ImGui::CollapsingHeader("Systems Info") ) {
		if ( ImGui::CollapsingHeader("Projectile System") ) {
			header = "Num entities: " + std::to_string(m_componentSystems.projectileSystem->getNumEntities());
			ImGui::Text(header.c_str());
		}
		if ( ImGui::CollapsingHeader("Render System") ) {
			header = "Num entities: " + std::to_string(m_componentSystems.modelSubmitSystem->getNumEntities());
			ImGui::Text(header.c_str());
		}
	}

	ImGui::End();
	return true;
}

void PerformanceTestState::shutDownPerformanceTestState() {

	// Show mouse cursor if hidden
	Input::HideCursor(false);

	ECS::Instance()->stopAllSystems();

	ECS::Instance()->destroyAllEntities();
}

// HERE BE DRAGONS
// Make sure things are updated in the correct order or things will behave strangely
void PerformanceTestState::updatePerTickComponentSystems(float dt) {
	m_currentlyReadingMask = 0;
	m_currentlyWritingMask = 0;
	m_runningSystemJobs.clear();
	m_runningSystems.clear();

	m_componentSystems.prepareUpdateSystem->update(); // HAS TO BE RUN BEFORE OTHER SYSTEMS WHICH USE TRANSFORM

	m_componentSystems.movementSystem->update(dt);
	m_componentSystems.speedLimitSystem->update();
	m_componentSystems.collisionSystem->update(dt);
	m_componentSystems.movementPostCollisionSystem->update(0.0f);


	// This can probably be used once the respective system developers 
	//	have checked their respective systems for proper component registration
	//runSystem(dt, m_componentSystems.physicSystem); // Needs to be updated before boundingboxes etc.

	// TODO: Investigate this
	runSystem(dt, m_componentSystems.gunSystem); // TODO: Order?
	runSystem(dt, m_componentSystems.projectileSystem);
	runSystem(dt, m_componentSystems.animationSystem);
	runSystem(dt, m_componentSystems.aiSystem);
	runSystem(dt, m_componentSystems.candleSystem);
	runSystem(dt, m_componentSystems.updateBoundingBoxSystem);
	runSystem(dt, m_componentSystems.octreeAddRemoverSystem);
	runSystem(dt, m_componentSystems.lifeTimeSystem);

	// Wait for all the systems to finish before starting the removal system
	for ( auto& fut : m_runningSystemJobs ) {
		fut.get();
	}

	// Will probably need to be called last
	m_componentSystems.entityAdderSystem->update();
	m_componentSystems.entityRemovalSystem->update();
}

void PerformanceTestState::updatePerFrameComponentSystems(float dt, float alpha) {
	// There is an imgui debug toggle to override lights
	if ( !m_disableLightComponents ) {
		m_lights.clearPointLights();
		//check and update all lights for all entities
		m_componentSystems.lightSystem->updateLights(&m_lights);
	}

	if ( m_showcaseProcGen ) {
		m_cam.setPosition(glm::vec3(100.f, 100.f, 100.f));
	}
	m_componentSystems.animationSystem->updatePerFrame();
	m_componentSystems.audioSystem->update(m_cam, dt, alpha);
}

void PerformanceTestState::runSystem(float dt, BaseComponentSystem* toRun) {
	bool started = false;
	while ( !started ) {
		// First check if the system can be run
		if ( !( m_currentlyReadingMask & toRun->getWriteBitMask() ).any() &&
			!( m_currentlyWritingMask & toRun->getReadBitMask() ).any() &&
			!( m_currentlyWritingMask & toRun->getWriteBitMask() ).any() ) {

			m_currentlyWritingMask |= toRun->getWriteBitMask();
			m_currentlyReadingMask |= toRun->getReadBitMask();
			started = true;
			m_runningSystems.push_back(toRun);
			m_runningSystemJobs.push_back(m_app->pushJobToThreadPool([this, dt, toRun] (int id) {toRun->update(dt); return toRun; }));

		} else {
			// Then loop through all futures and see if any of them are done
			for ( int i = 0; i < m_runningSystemJobs.size(); i++ ) {
				if ( m_runningSystemJobs[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready ) {
					auto doneSys = m_runningSystemJobs[i].get();

					m_runningSystemJobs.erase(m_runningSystemJobs.begin() + i);
					i--;

					m_currentlyWritingMask ^= doneSys->getWriteBitMask();
					m_currentlyReadingMask ^= doneSys->getReadBitMask();

					int toRemoveIndex = -1;
					for ( int j = 0; j < m_runningSystems.size(); j++ ) {
						// Currently just compares memory adresses (if they point to the same location they're the same object)
						if ( m_runningSystems[j] == doneSys )
							toRemoveIndex = j;
					}

					m_runningSystems.erase(m_runningSystems.begin() + toRemoveIndex);

					// Since multiple systems can read from components concurrently, currently best solution I came up with
					for ( auto _sys : m_runningSystems ) {
						m_currentlyReadingMask |= _sys->getReadBitMask();
					}
				}
			}
		}
	}
}

Entity::SPtr PerformanceTestState::createCandleEntity(const std::string& name, Model* lightModel, Model* bbModel, glm::vec3 lightPos) {
	//creates light with model and pointlight
	auto e = ECS::Instance()->createEntity(name.c_str());
	e->addComponent<CandleComponent>();
	e->addComponent<ModelComponent>(lightModel);
	e->addComponent<TransformComponent>(lightPos);
	e->addComponent<BoundingBoxComponent>(bbModel);
	e->addComponent<CollidableComponent>();
	PointLight pl;
	pl.setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	pl.setPosition(glm::vec3(lightPos.x, lightPos.y + .37f, lightPos.z));
	pl.setAttenuation(.0f, 0.1f, 0.02f);
	pl.setIndex(m_currLightIndex);
	m_currLightIndex++;
	e->addComponent<LightComponent>(pl);

	return e;
}

void PerformanceTestState::loadAnimations() {
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();
	m_app->getResourceManager().loadModel("AnimationTest/walkTri.fbx", shader, ResourceManager::ImporterType::SAIL_FBXSDK);
	//animatedModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");




}

void PerformanceTestState::initAnimations() {
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();



	auto animationEntity2 = ECS::Instance()->createEntity("animatedModel2");
	animationEntity2->addComponent<TransformComponent>();
	animationEntity2->getComponent<TransformComponent>()->translate(-5, 0, 0);
	animationEntity2->getComponent<TransformComponent>()->translate(100.f, 100.f, 100.f);
	animationEntity2->addComponent<ModelComponent>(&m_app->getResourceManager().getModelCopy("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<ModelComponent>()->getModel()->setIsAnimated(true);
	animationEntity2->addComponent<AnimationComponent>(&m_app->getResourceManager().getAnimationStack("AnimationTest/walkTri.fbx"));
	animationEntity2->getComponent<AnimationComponent>()->currentAnimation = animationEntity2->getComponent<AnimationComponent>()->getAnimationStack()->getAnimation(0);

}

void PerformanceTestState::populateScene(Model* characterModel, Model* lightModel, Model* bbModel, Model* projectileModel, Shader* shader) {
	/* 13 characters that are constantly shooting their guns */
	for ( int i = 0; i < 13; i++ ) {
		float spawnOffsetX = -24.f + float(i) * 2.f;
		float spawnOffsetZ = float(i) * 1.3f;
		auto e = ECS::Instance()->createEntity("Performance Test Entity " + std::to_string(i));

		Model* characterModel = &m_app->getResourceManager().getModelCopy("walkTri.fbx", shader);
		characterModel->getMesh(0)->getMaterial()->setMetalnessScale(0.0f);
		characterModel->getMesh(0)->getMaterial()->setRoughnessScale(0.217f);
		characterModel->getMesh(0)->getMaterial()->setAOScale(0.0f);
		characterModel->getMesh(0)->getMaterial()->setAlbedoTexture("sponza/textures/character1texture.tga");
		characterModel->setIsAnimated(true);

		e->addComponent<ModelComponent>(characterModel);
		auto animStack = &m_app->getResourceManager().getAnimationStack("walkTri.fbx");
		auto animComp = e->addComponent<AnimationComponent>(animStack);
		animComp->currentAnimation = animStack->getAnimation(0);
		animComp->animationTime = float(i) / animComp->currentAnimation->getMaxAnimationTime();
		e->addComponent<TransformComponent>(glm::vec3(105.543f + spawnOffsetX, 0.f, 99.5343f + spawnOffsetZ), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(bbModel)->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));
		e->addComponent<CollidableComponent>();
		e->addComponent<MovementComponent>();
		e->addComponent<SpeedLimitComponent>();
		e->addComponent<CollisionComponent>();
		e->addComponent<GunComponent>(projectileModel, bbModel);

		/* Audio */
		e->addComponent<AudioComponent>();
		Audio::SoundInfo sound {};
		sound.fileName = "../Audio/guitar.wav";
		sound.soundEffectLength = 104.0f;
		sound.volume = 1.0f;
		sound.playOnce = false;
		sound.positionalOffset = { 0.f, 1.2f, 0.f };
		sound.isPlaying = true; // Start playing the sound immediately
		e->getComponent<AudioComponent>()->defineSound(Audio::SoundType::AMBIENT, sound);

		// Add candle
		if ( i != 12 ) {
			auto candleEntity = createCandleEntity("Candle Entity " + std::to_string(i), lightModel, bbModel, glm::vec3(0.f, 10.f, 0.f));
			candleEntity->getComponent<CandleComponent>()->setOwner(e->getID());
			e->addChildEntity(candleEntity);
		}

		/* Movement */
		e->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.0f, -9.8f, 0.0f);
		e->getComponent<SpeedLimitComponent>()->maxSpeed = 6.f;

		m_performanceEntities.push_back(e);
	}
}

void PerformanceTestState::createBots(Model* boundingBoxModel, Model* characterModel, Model* projectileModel, Model* lightModel) {
	int botCount = m_app->getStateStorage().getLobbyToGameData()->botCount;

	if (botCount < 0) {
		botCount = 0;
	}

	for (size_t i = 0; i < botCount; i++) {
		auto e = EntityFactory::CreateBot(boundingBoxModel, characterModel, glm::vec3(2.f * (i + 1), 10.f, 0.f), lightModel, m_currLightIndex++, m_componentSystems.aiSystem->getNodeSystem());
	}
}

void PerformanceTestState::createLevel(Shader* shader, Model* boundingBoxModel) {
	std::string tileTex = "sponza/textures/tileTexture1.tga";
	Application::getInstance()->getResourceManager().loadTexture(tileTex);



	//Load tileset for world
	Model* tileFlat = &m_app->getResourceManager().getModel("Tiles/tileFlat.fbx", shader);
	tileFlat->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	Model* tileEnd = &m_app->getResourceManager().getModel("Tiles/tileEnd.fbx", shader);
	tileEnd->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	Model* tileDoor = &m_app->getResourceManager().getModel("Tiles/tileDoor.fbx", shader);
	tileDoor->getMesh(0)->getMaterial()->setAlbedoTexture(tileTex);

	std::vector<Model*> tileModels;
	tileModels.resize(TileModel::NUMBOFMODELS);
	tileModels[TileModel::ROOM_FLOOR] = tileFlat;
	tileModels[TileModel::ROOM_WALL] = tileEnd;
	tileModels[TileModel::ROOM_DOOR] = tileDoor;

	tileModels[TileModel::CORRIDOR_FLOOR] = tileFlat;
	tileModels[TileModel::CORRIDOR_WALL] = tileEnd;
	tileModels[TileModel::CORRIDOR_DOOR] = tileDoor;



	// Create the level generator system and put it into the datatype.
	auto map = ECS::Instance()->createEntity("Map");
	map->addComponent<MapComponent>();
	ECS::Instance()->addAllQueuedEntities();
	m_componentSystems.levelGeneratorSystem->generateMap();
	m_componentSystems.levelGeneratorSystem->createWorld(tileModels, boundingBoxModel);

}