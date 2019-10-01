#include "GameState.h"
#include "imgui.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/systems/candles/CandleSystem.h"
#include "Sail/entities/systems/entityManagement/EntityAdderSystem.h"
#include "Sail/entities/systems/entityManagement/EntityRemovalSystem.h"
#include "Sail/entities/systems/lifetime/LifeTimeSystem.h"
#include "Sail/entities/systems/light/LightSystem.h"
#include "Sail/entities/systems/gameplay/ai/AiSystem.h"
#include "Sail/entities/systems/gameplay/GunSystem.h"
#include "Sail/entities/systems/Gameplay/ProjectileSystem.h"
#include "Sail/entities/systems/Graphics/AnimationSystem.h"
#include "Sail/entities/systems/physics/OctreeAddRemoverSystem.h"
#include "Sail/entities/systems/physics/PhysicSystem.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "Sail/entities/systems/prepareUpdate/PrepareUpdateSystem.h"
#include "Sail/entities/systems/Input/GameInputSystem.h"
#include "Sail/entities/systems/Audio/AudioSystem.h"
#include "Sail/entities/systems/render/RenderSystem.h"
#include "Sail/TimeSettings.h"
#include "Sail/utils/GameDataTracker.h"
#include "Network/NWrapperSingleton.h"

#include <sstream>
#include <iomanip>

// Uncomment to use forward rendering
//#define DISABLE_RT

GameState::GameState(StateStack& stack)
: State(stack)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_cc(true)
, m_profiler(true)
, m_disableLightComponents(false)
{
#ifdef _DEBUG
#pragma region TESTCASES
	m_cc.addCommand(std::string("Save"), [&]() { return std::string("saved"); });
	m_cc.addCommand(std::string("Test <int>"), [&](int in) { return std::string("test<int>"); });
	m_cc.addCommand(std::string("Test <float>"), [&](float in) { return std::string("test<float>"); });
	m_cc.addCommand(std::string("Test <string>"), [&](std::string in) { return std::string("test<string>"); });
	m_cc.addCommand(std::string("Test <int> <int> <int>"/*...*/), [&](std::vector<int> in) {return std::string("test<std::vector<int>"); });
	m_cc.addCommand(std::string("Test <float> <float> <float>"/*...*/), [&](std::vector<float> in) {return std::string("test<std::vector<float>"); });
#pragma endregion


	m_cc.addCommand(std::string("AddCube"), [&]() {
		return createCube(m_cam.getPosition());
		});
	m_cc.addCommand(std::string("AddCube <int> <int> <int>"), [&](std::vector<int> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
		});
	m_cc.addCommand(std::string("AddCube <float> <float> <float>"), [&](std::vector<float> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
		});
#endif

	// Get the Application instance
	m_app = Application::getInstance();

	//----Octree creation----
	//Wireframe shader
	auto* wireframeShader = &m_app->getResourceManager().getShaderSet<WireframeShader>();

	//Wireframe bounding box model
	Model* boundingBoxModel = &m_app->getResourceManager().getModel("boundingBox.fbx", wireframeShader);
	boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create octree
	m_octree = SAIL_NEW Octree(boundingBoxModel);
	//-----------------------

	/*
		Create a PhysicSystem
		If the game developer does not want to add the systems like this,
		this call could be moved inside the default constructor of ECS,
		assuming each system is included in ECS.cpp instead of here
	*/
	m_componentSystems.physicSystem = ECS::Instance()->createSystem<PhysicSystem>();
	m_componentSystems.physicSystem->provideOctree(m_octree);

	m_componentSystems.animationSystem = ECS::Instance()->createSystem<AnimationSystem>();

	//Create system for updating bounding box
	m_componentSystems.updateBoundingBoxSystem = ECS::Instance()->createSystem<UpdateBoundingBoxSystem>();

	//Create system for handling octree
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

	//Create system for the lights
	m_componentSystems.lightSystem = ECS::Instance()->createSystem<LightSystem>();
	
	//Create system for the candles
	m_componentSystems.candleSystem = ECS::Instance()->createSystem<CandleSystem>();

	//Create system which prepares each new update
	m_componentSystems.prepareUpdateSystem = ECS::Instance()->createSystem<PrepareUpdateSystem>();

	m_componentSystems.gunSystem = ECS::Instance()->createSystem<GunSystem>();
	
	m_componentSystems.projectileSystem = ECS::Instance()->createSystem<ProjectileSystem>();

	// Create system for handling and updating sounds
	m_componentSystems.audioSystem = ECS::Instance()->createSystem<AudioSystem>();

	m_componentSystems.renderSystem = ECS::Instance()->createSystem<RenderSystem>();

	//Create system for input
	m_componentSystems.gameInputSystem = ECS::Instance()->createSystem<GameInputSystem>();
	m_componentSystems.gameInputSystem->initialize(&m_cam);

	// Textures needs to be loaded before they can be used
	// TODO: automatically load textures when needed so the following can be removed
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/arenaBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/barrierBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/containerBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/rampBasicTexture.tga");
	//Application::getInstance()->getResourceManager().loadTexture("sponza/textures/boxOrientationTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/candleBasicTexture.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/character1texture.tga");




	// Add a directional light
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));

	// Set up the scene
	//addSkybox(L"skybox_space_512.dds"); //TODO

	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);
	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

#ifdef DISABLE_RT
	auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();
	(*Application::getInstance()->getRenderWrapper()).changeRenderer(1);
	m_componentSystems.renderSystem->refreshRenderer();
	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);
#else
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();
#endif

	// Create/load models

	Model* cubeModel = &m_app->getResourceManager().getModel("cubeWidth1.fbx", shader);
	cubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));

	Model* arenaModel = &m_app->getResourceManager().getModel("arenaBasic.fbx", shader);
	arenaModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/arenaBasicTexture.tga");

	Model* barrierModel = &m_app->getResourceManager().getModel("barrierBasic.fbx", shader);
	barrierModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/barrierBasicTexture.tga");

	Model* containerModel = &m_app->getResourceManager().getModel("containerBasic.fbx", shader);
	containerModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/containerBasicTexture.tga");

	Model* rampModel = &m_app->getResourceManager().getModel("rampBasic.fbx", shader);
	rampModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/rampBasicTexture.tga");

	Model* lightModel = &m_app->getResourceManager().getModel("candleExported.fbx", shader);
	lightModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/candleBasicTexture.tga");

	Model* characterModel = &m_app->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");


	// Player creation
	auto player = ECS::Instance()->createEntity("player");
	
	// TODO: Only used for AI, should be removed once AI can target player in a better way.
	m_player = player.get();

	player->addComponent<PlayerComponent>();
	player->addComponent<TransformComponent>();
	player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(0.0f, 0.f, 0.f));

	player->addComponent<PhysicsComponent>();
	player->getComponent<PhysicsComponent>()->constantAcceleration = glm::vec3(0.0f, -9.8f, 0.0f);
	player->getComponent<PhysicsComponent>()->maxSpeed = 6.0f;

	// Give player a bounding box
	player->addComponent<BoundingBoxComponent>(boundingBoxModel);
	player->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));

	// Temporary projectile model for the player's gun
	player->addComponent<GunComponent>(cubeModel, boundingBoxModel);


	player->addComponent<AudioComponent>();
	player->getComponent<AudioComponent>()->defineSound(SoundType::RUN, "../Audio/footsteps_1.wav", 0.94f, false);
	player->getComponent<AudioComponent>()->defineSound(SoundType::JUMP, "../Audio/jump.wav", 0.0f, true);


	// Create candle for the player
	m_currLightIndex = 0;
	auto e = createCandleEntity("PlayerCandle", lightModel, boundingBoxModel, glm::vec3(0.f, 2.f, 0.f));
	e->addComponent<RealTimeComponent>(); // Player candle will have its position updated each frame
	e->getComponent<CandleComponent>()->setOwner(player->getID());
	player->addChildEntity(e);

	// Set up camera
	m_cam.setPosition(glm::vec3(1.6f, 1.8f, 10.f));
	m_cam.lookAt(glm::vec3(0.f));
	player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(1.6f, 0.9f, 10.f));



	// Inform CandleSystem of the player
	m_componentSystems.candleSystem->setPlayerEntityID(player->getID());


	/*
		Creation of entities
	*/

	
	/*Model* animatedModel = &m_app->getResourceManager().getModel("walkingAnimationBaked.fbx", shader); 
	AnimationStack* animationStack = &m_app->getResourceManager().getAnimationStack("walkingAnimationBaked.fbx");
	animatedModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

	auto animationEntity = ECS::Instance()->createEntity("animatedModel");
	animationEntity->addComponent<TransformComponent>();
	animationEntity->addComponent<ModelComponent>(animatedModel);
	animationEntity->addComponent<AnimationComponent>(animationStack);
	animationEntity->getComponent<AnimationComponent>()->currentAnimation = animationStack->getAnimation(0);

	*/

	{
		auto e = ECS::Instance()->createEntity("Arena");
		e->addComponent<ModelComponent>(arenaModel);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		

		e = ECS::Instance()->createEntity("Map_Barrier1");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<TransformComponent>(glm::vec3(-16.15f*0.3f, 0.f, 3.83f*0.3f), glm::vec3(0.f, -0.79f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Barrier2");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<TransformComponent>(glm::vec3(-4.54f*0.3f, 0.f, 8.06f *0.3f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		

		e = ECS::Instance()->createEntity("Map_Barrier3");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<TransformComponent>(glm::vec3(8.46f *0.3f, 0.f, 8.06f *0.3f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		

		e = ECS::Instance()->createEntity("Map_Container1");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(6.95f *0.3f, 0.f, 25.f *0.3f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Container2");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(-25.f*0.3f, 0.f, 12.43f*0.3f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		

		e = ECS::Instance()->createEntity("Map_Container3");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(-25.f*0.3f, 2.4f, -7.73f*0.3f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		

		e = ECS::Instance()->createEntity("Map_Container4");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(-19.67f*0.3f, 0.f, -24.83f*0.3f), glm::vec3(0.f, 0.79f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Container5");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(-0.f, 0.f, -14.f*0.3f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Container6");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(24.20f*0.3f, 0.f, -8.f*0.3f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();


		e = ECS::Instance()->createEntity("Map_Container7");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(24.2f*0.3f, 2.4f, -22.8f*0.3f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Container8");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<TransformComponent>(glm::vec3(24.36f*0.3f, 0.f, -32.41f*0.3f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	

		e = ECS::Instance()->createEntity("Map_Ramp1");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(5.2f *0.3f, 0.f, -32.25f *0.3f), glm::vec3(0.f, 3.14f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	
		e = ECS::Instance()->createEntity("Map_Ramp2");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(15.2f*0.3f, 2.4f, -32.25f*0.3f), glm::vec3(0.f, 3.14f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
	
		e = ECS::Instance()->createEntity("Map_Ramp3");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(24.f*0.3f, 2.4f, -5.5f*0.3f), glm::vec3(0.f, 4.71f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		e = ECS::Instance()->createEntity("Map_Ramp4");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(24.f*0.3f, 0.f, 9.f*0.3f), glm::vec3(0.f, 4.71f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		e = ECS::Instance()->createEntity("Map_Ramp5");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(-16.f*0.3f, 0.f, 20.f*0.3f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		e = ECS::Instance()->createEntity("Map_Ramp6");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<TransformComponent>(glm::vec3(-34.f*0.3f, 0.f, 20.f*0.3f), glm::vec3(0.f, 3.14f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();

		e = ECS::Instance()->createEntity("Character");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<PhysicsComponent>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();

		int botCount = m_app->getStateStorage().getLobbyToGameStateData()->botCount;
		for (size_t i = 0; i < botCount; i++) {
			e = ECS::Instance()->createEntity("AiCharacter");
			e->addComponent<ModelComponent>(characterModel);
			e->addComponent<TransformComponent>(glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
			e->addComponent<BoundingBoxComponent>(boundingBoxModel);
			e->addComponent<CollidableComponent>();
			e->addComponent<PhysicsComponent>();
			e->addComponent<AiComponent>();
			e->addComponent<GunComponent>(cubeModel, boundingBoxModel);
			e->addChildEntity(createCandleEntity("AiCandle", lightModel, boundingBoxModel, glm::vec3(0.f, 2.f, 0.f)));

			e = createCandleEntity("Map_Candle1", lightModel, boundingBoxModel, glm::vec3(0.f, 0.0f, 0.f));
		}
		/*
		e = ECS::Instance()->createEntity("AiCharacter");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(boundingBoxModel);
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		e->addComponent<AiComponent>();
		e->addComponent<GunComponent>(cubeModel, boundingBoxModel);.
		e->addChildEntity(createCandleEntity("AiCandle", lightModel, boundingBoxModel, glm::vec3(0.f, 2.f, 0.f)));
		


		e = createCandleEntity("Map_Candle1", lightModel, boundingBoxModel, glm::vec3(0.f, 0.0f, 0.f));*/


#ifdef _DEBUG
		// Candle1 holds all lights you can place in debug...
		m_componentSystems.lightSystem->setDebugLightListEntity("Map_Candle1");
#endif




		m_virtRAMHistory = SAIL_NEW float[100];
		m_physRAMHistory = SAIL_NEW float[100];
		m_vramUsageHistory = SAIL_NEW float[100];
		m_cpuHistory = SAIL_NEW float[100];
		m_frameTimesHistory = SAIL_NEW float[100];
	}





	auto nodeSystemCube = ModelFactory::CubeModel::Create(glm::vec3(0.1f), shader);
#ifdef _DEBUG_NODESYSTEM
	m_componentSystems.aiSystem->initNodeSystem(nodeSystemCube.get(), m_octree, wireframeShader);
#else
	m_componentSystems.aiSystem->initNodeSystem(nodeSystemCube.get(), m_octree);
#endif
}

GameState::~GameState() {
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

	if (Input::WasKeyJustPressed(KeyBinds::showBoundingBoxes)) {
		m_componentSystems.renderSystem->toggleHitboxes();
	}

	//Test ray intersection
	if (Input::IsKeyPressed(KeyBinds::testRayIntersection)) {
		Octree::RayIntersectionInfo tempInfo;
		m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
		if (tempInfo.entity) {
			Logger::Log("Ray intersection with " + tempInfo.entity->getName() + ", " + std::to_string(tempInfo.closestHit) + " meters away");
		}
	}

	// Toggle ai following the player
	if (Input::WasKeyJustPressed(KeyBinds::toggleAIFollowing)) {
		auto entities = m_componentSystems.aiSystem->getEntities();
		for ( int i = 0; i < entities.size(); i++ ) {
			auto aiComp = entities[i]->getComponent<AiComponent>();
			if ( aiComp->entityTarget == nullptr ) {
				
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

	if (Input::IsKeyPressed(KeyBinds::setDirectionalLight)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}
	if (Input::WasKeyJustPressed(KeyBinds::toggleConsole)) {
		m_cc.toggle();
		m_profiler.toggle();
	}

	// Reload shaders
	if (Input::WasKeyJustPressed(KeyBinds::reloadShader)) {
		m_app->getResourceManager().reloadShader<MaterialShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
	}  

	// Lights the selected candle
	if (Input::WasKeyJustPressed(KeyBinds::lightCandle1)) {
		m_componentSystems.candleSystem->lightCandle("Map_Candle1");
	}
	if (Input::WasKeyJustPressed(KeyBinds::lightCandle2)) {
		m_componentSystems.candleSystem->lightCandle("Map_Candle2");
	}

	if (Input::WasKeyJustPressed(KeyBinds::showInGameMenu)) {
		if (m_paused) {
			Input::HideCursor(!Input::IsCursorHidden());
			m_paused = false;
			requestStackPop();
		}
		else if(!m_paused){
			if (Input::IsCursorHidden()) {
				Input::HideCursor(!Input::IsCursorHidden());
			}
			m_paused = true;
			requestStackPush(States::Pause);

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
	EventHandler::dispatch<PlayerCandleHitEvent>(event, SAIL_BIND_EVENT(&GameState::onPlayerCandleHit));

	return true;
}

bool GameState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool GameState::onPlayerCandleHit(PlayerCandleHitEvent& event) {
	this->requestStackPop();
	this->requestStackPush(States::EndGame);
	m_poppedThisFrame = true;
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
	m_componentSystems.renderSystem->draw(m_cam, alpha);

	return true;
}

bool GameState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	renderImguiConsole(dt);
	renderImguiProfiler(dt);
	renderImGuiRenderSettings(dt);
	renderImGuiLightDebug(dt);

	return false;
}

bool GameState::renderImguiConsole(float dt) {
	bool open = m_cc.windowOpen();
	if (open) {
		static char buf[256] = "";
		if (ImGui::Begin("Console", &open)) {
			m_cc.windowState(open);
			std::string txt = "test";
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
			
			for (int i = 0; i < m_cc.getLog().size(); i++) {
				ImGui::TextUnformatted(m_cc.getLog()[i].c_str());
			}

			ImGui::EndChild();
			ImGui::Separator();
			bool reclaim_focus = false;

			m_cc.getTextField().copy(buf, m_cc.getTextField().size() + 1);
			buf[m_cc.getTextField().size()] = '\0';

			std::string original = m_cc.getTextField();
			bool exec = ImGui::InputText("", buf, IM_ARRAYSIZE(buf),
				ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();
			if (exec || ImGui::Button("Execute", ImVec2(0, 0))) {
				if (m_cc.execute()) {

				}
				reclaim_focus = true;
			} else {
				m_cc.setTextField(std::string(buf));
			}
			ImGui::End();
		} else {
			ImGui::End();
		}
	}


	return false;
}

bool GameState::renderImguiProfiler(float dt) {
	bool open = m_profiler.windowOpen();
	if (open) {
		if (ImGui::Begin("Profiler", &open)) {
			m_profiler.windowState(open);
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
			if (ImGui::CollapsingHeader("CPU Graph")) {
				header = "\n\n\n" + m_cpuCount + "(%)";
				ImGui::PlotLines(header.c_str(), m_cpuHistory, 100, 0, "", 0.f, 100.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Frame Times Graph")) {
				header = "\n\n\n" + m_ftCount + "(s)";
				ImGui::PlotLines(header.c_str(), m_frameTimesHistory, 100, 0, "", 0.f, 0.01f, ImVec2(0, 100));
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
			if (m_profilerTimer > 0.2f) {
				m_profilerTimer = 0.f;
				if (m_profilerCounter < 100) {

					m_virtRAMHistory[m_profilerCounter] = m_profiler.virtMemUsage();
					m_physRAMHistory[m_profilerCounter] = m_profiler.workSetUsage();
					m_vramUsageHistory[m_profilerCounter] = m_profiler.vramUsage();
					m_frameTimesHistory[m_profilerCounter] = dt;
					m_cpuHistory[m_profilerCounter++] = m_profiler.processUsage();
					m_virtCount = std::to_string(m_profiler.virtMemUsage());
					m_physCount = std::to_string(m_profiler.workSetUsage());
					m_vramUCount = std::to_string(m_profiler.vramUsage());
					m_cpuCount = std::to_string(m_profiler.processUsage());
					m_ftCount = std::to_string(dt);

				} else {
					float* tempFloatArr = SAIL_NEW float[100];
					std::copy(m_virtRAMHistory + 1, m_virtRAMHistory + 100, tempFloatArr);
					tempFloatArr[99] = m_profiler.virtMemUsage();
					delete m_virtRAMHistory;
					m_virtRAMHistory = tempFloatArr;
					m_virtCount = std::to_string(m_profiler.virtMemUsage());

					float* tempFloatArr1 = SAIL_NEW float[100];
					std::copy(m_physRAMHistory + 1, m_physRAMHistory + 100, tempFloatArr1);
					tempFloatArr1[99] = m_profiler.workSetUsage();
					delete m_physRAMHistory;
					m_physRAMHistory = tempFloatArr1;
					m_physCount = std::to_string(m_profiler.workSetUsage());

					float* tempFloatArr3 = SAIL_NEW float[100];
					std::copy(m_vramUsageHistory + 1, m_vramUsageHistory + 100, tempFloatArr3);
					tempFloatArr3[99] = m_profiler.vramUsage();
					delete m_vramUsageHistory;
					m_vramUsageHistory = tempFloatArr3;
					m_vramUCount = std::to_string(m_profiler.vramUsage());

					float* tempFloatArr4 = SAIL_NEW float[100];
					std::copy(m_cpuHistory + 1, m_cpuHistory + 100, tempFloatArr4);
					tempFloatArr4[99] = m_profiler.processUsage();
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
			ImGui::SliderFloat3("Position##", &position[0], -40.f, 40.0f);
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

void GameState::shutDownGameState() {

	// Show mouse cursor if hidden
	if (Input::IsCursorHidden()) {
		Input::HideCursor(!Input::IsCursorHidden());
	}

	// Reset network
	NWrapperSingleton::getInstance().resetNetwork();

	// Clear all entities
	ECS::Instance()->destroyAllEntities();
	
	// Clear all neccesary systems
	m_componentSystems.gameInputSystem->clean();

}

// HERE BE DRAGONS
// Make sure things are updated in the correct order or things will behave strangely
void GameState::updatePerTickComponentSystems(float dt) {
	m_currentlyReadingMask = 0;
	m_currentlyWritingMask = 0;
	m_runningSystemJobs.clear();
	m_runningSystems.clear();
	
	m_componentSystems.prepareUpdateSystem->update(dt); // HAS TO BE RUN BEFORE OTHER SYSTEMS
	
	m_componentSystems.entityAdderSystem->update(0.0f);

	m_componentSystems.physicSystem->update(dt);
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
	runSystem(dt, m_componentSystems.audioSystem);

	// Wait for all the systems to finish before starting the removal system
	for ( auto& fut : m_runningSystemJobs ) {
		fut.get();
	}

	// Will probably need to be called last
	m_componentSystems.entityRemovalSystem->update(0.0f);

	if (m_poppedThisFrame) {
		shutDownGameState();
	}
}

void GameState::updatePerFrameComponentSystems(float dt, float alpha) {
	// Updates the camera
	m_componentSystems.gameInputSystem->update(dt, alpha);

	m_componentSystems.gameInputSystem->updateCameraPosition(alpha);

	// There is an imgui debug toggle to override lights
	if (!m_disableLightComponents) {
		m_lights.clearPointLights();
		//check and update all lights for all entities
		m_componentSystems.lightSystem->updateLights(&m_lights);
	}

	m_componentSystems.audioSystem->update(dt);
}

void GameState::runSystem(float dt, BaseComponentSystem* toRun) {
	bool started = false;
	while ( !started ) {
		// First check if the system can be run
		if ( !(m_currentlyReadingMask & toRun->getWriteBitMask()).any() && 
			!(m_currentlyWritingMask & toRun->getReadBitMask()).any() &&
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

Entity::SPtr GameState::createCandleEntity(const std::string& name, Model* lightModel, Model* bbModel, glm::vec3 lightPos) {
	//creates light with model and pointlight
	auto e = ECS::Instance()->createEntity(name.c_str());
	e->addComponent<CandleComponent>();
	e->addComponent<ModelComponent>(lightModel);
	e->addComponent<TransformComponent>(lightPos);
	e->addComponent<BoundingBoxComponent>(bbModel);
	e->addComponent<CollidableComponent>();
	PointLight pl;
	pl.setColor(glm::vec3(0.2f, 0.2f, 0.2f));
	pl.setPosition(glm::vec3(lightPos.x, lightPos.y + .37f, lightPos.z));
	pl.setAttenuation(.0f, 0.1f, 0.02f);
	pl.setIndex(m_currLightIndex++);
	e->addComponent<LightComponent>(pl);

	return e;
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

