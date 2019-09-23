#include "GameState.h"
#include "imgui.h"
#include "..//Sail/src/Sail/entities/systems/physics/PhysicSystem.h"
#include "..//Sail/src/Sail/entities/systems/Graphics/AnimationSystem.h"
#include "..//Sail/src/Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "..//Sail/src/Sail/entities/systems/physics/OctreeAddRemoverSystem.h"
#include "..//Sail/src/Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include <sstream>
#include <iomanip>
#include "Sail/ai/pathfinding/NodeSystem.h"

GameState::GameState(StateStack& stack)
: State(stack)
//, m_cam(20.f, 20.f, 0.1f, 5000.f)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
//, m_camController(&m_cam)
, m_playerController(&m_cam, &m_scene)
, m_cc(true)
, m_profiler(true)
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
	m_boundingBoxModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), wireframeShader);
	m_boundingBoxModel->getMesh(0)->getMaterial()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//Create octree
	m_octree = SAIL_NEW Octree(&m_scene, m_boundingBoxModel.get());
	//-----------------------

	/*
		Create a PhysicSystem
		If the game developer does not want to add the systems like this,
		this call could be moved inside the default constructor of ECS,
		assuming each system is included in ECS.cpp instead of here
	*/
	ECS::Instance()->createSystem<PhysicSystem>();
	ECS::Instance()->getSystem<PhysicSystem>()->provideOctree(m_octree);
	m_componentSystems.physicSystem = ECS::Instance()->getSystem<PhysicSystem>();
	m_componentSystems.animationSystem = ECS::Instance()->createSystem<AnimationSystem>();

	//Create system for updating bounding box
	ECS::Instance()->createSystem<UpdateBoundingBoxSystem>();
	m_componentSystems.updateBoundingBoxSystem = ECS::Instance()->getSystem<UpdateBoundingBoxSystem>();

	//Create system for handeling octree
	ECS::Instance()->createSystem<OctreeAddRemoverSystem>();
	ECS::Instance()->getSystem<OctreeAddRemoverSystem>()->provideOctree(m_octree);
	m_componentSystems.octreeAddRemoverSystem = ECS::Instance()->getSystem<OctreeAddRemoverSystem>();

	// This was moved out from the PlayerController constructor
	// since the PhysicSystem needs to be created first
	// (or the PhysicsComponent needed to be detached and reattached
	m_playerController.getEntity()->addComponent<PhysicsComponent>();
	m_playerController.getEntity()->getComponent<PhysicsComponent>()->acceleration = glm::vec3(0.0f, -30.0f, 0.0f);


	//m_scene = std::make_unique<Scene>(AABB(glm::vec3(-100.f, -100.f, -100.f), glm::vec3(100.f, 100.f, 100.f)));

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



	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 4.7f, 7.4f));
	//m_camController.lookAt(glm::vec3(0.f));
	m_cam.lookAt(glm::vec3(0.f));
	m_playerController.getEntity()->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(1.6f, 4.7f, 7.4f));
	

	// Add a directional light
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));
	// Add four point lights
	{
		//PointLight pl;
		//pl.setAttenuation(.0f, 0.1f, 0.02f);
		//pl.setColor(glm::vec3(1.f, 1.f, 1.f));
		//pl.setPosition(glm::vec3(3.0f, 3.1f, 3.0f));
		//pl.setIndex(0);
		//m_lights.addPointLight(pl);

		//pl.setColor(glm::vec3(1.f,1.f,1.f));
		//pl.setPosition(glm::vec3(1.0f, 3.1f, 1.0f));
		//pl.setIndex(1);
		//m_lights.addPointLight(pl);


		//pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		//pl.setPosition(glm::vec3(-4.0f, 0.1f, 4.0f));
		//m_lights.addPointLight(pl);

		//pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		//pl.setPosition(glm::vec3(4.0f, 0.1f, 4.0f));
		//m_lights.addPointLight(pl);

		//pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		//pl.setPosition(glm::vec3(4.0f, 0.1f, -4.0f));
		//m_lights.addPointLight(pl);
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
	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(50.f), shader, glm::vec2(3.0f));
	m_planeModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	m_planeModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	m_planeModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");

	Model* fbxModel = &m_app->getResourceManager().getModel("sphere.fbx", shader);
	fbxModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/spnza_bricks_a_diff.tga");
	fbxModel->getMesh(0)->getMaterial()->setNormalTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	fbxModel->getMesh(0)->getMaterial()->setSpecularTexture("sponza/textures/spnza_bricks_a_spec.tga");

	Model* arenaModel = &m_app->getResourceManager().getModel("arenaBasic.fbx", shader);
	arenaModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/arenaBasicTexture.tga");

	Model* barrierModel = &m_app->getResourceManager().getModel("barrierBasic.fbx", shader);
	barrierModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/barrierBasicTexture.tga");

	Model* containerModel = &m_app->getResourceManager().getModel("containerBasic.fbx", shader);
	containerModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/containerBasicTexture.tga");

	Model* rampModel = &m_app->getResourceManager().getModel("rampBasic.fbx", shader);
	rampModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/rampBasicTexture.tga");

	//For testing purposes only
	//Model* boxOrientation = &m_app->getResourceManager().getModel("boxOrientation.fbx", shader);
	//boxOrientation->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/boxOrientationTexture.tga");
	//Model* lrTest= &m_app->getResourceManager().getModel("lrTest.fbx", shader);

	Model* lightModel = &m_app->getResourceManager().getModel("candleExported.fbx", shader);
	lightModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/candleBasicTexture.tga");

	Model* characterModel = &m_app->getResourceManager().getModel("character1.fbx", shader);
	characterModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

	//Give player a bounding box
	m_playerController.getEntity()->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
	m_playerController.getEntity()->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.7f, 3.0f, 0.7f));
	m_scene.addEntity(m_playerController.getEntity());

	// Temporary projectile model for the player's gun
	m_playerController.setProjectileModels(m_cubeModel.get(), m_boundingBoxModel.get());

	/*
		Creation of entities
	*/

	
	Model* animatedModel = &m_app->getResourceManager().getModel("walkingAnimationBaked.fbx", shader); 
	AnimationStack* animationStack = &m_app->getResourceManager().getAnimationStack("walkingAnimationBaked.fbx");
	animatedModel->getMesh(0)->getMaterial()->setDiffuseTexture("sponza/textures/character1texture.tga");

	auto animationEntity = ECS::Instance()->createEntity("animatedModel");
	animationEntity->addComponent<TransformComponent>();
	animationEntity->addComponent<ModelComponent>(animatedModel);
	animationEntity->addComponent<AnimationComponent>(animationStack);
	animationEntity->getComponent<AnimationComponent>()->currentAnimation = animationStack->getAnimation(0);

	m_scene.addEntity(animationEntity);

	// STATIC ENTITIES (never added/deleted/modified during runtime)
	// Use .addStaticEntity() and StaticMatrixComponent instead of TransformComponent since static objects's transforms 
	// don't need to be interpolated between updates.
	{
		auto e = ECS::Instance()->createEntity("Arena");
		e->addComponent<ModelComponent>(arenaModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Barrier1");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-16.15f, 0.f, 3.83f), glm::vec3(0.f, -0.79f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Barrier2");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-4.54f, 0.f, 8.06f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Barrier3");
		e->addComponent<ModelComponent>(barrierModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(8.46f, 0.f, 8.06f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container1");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(6.95f, 0.f, 25.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container2");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-25.f, 0.f, 12.43f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container3");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-25.f, 8.f, -7.73f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container4");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-19.67f, 0.f, -24.83f), glm::vec3(0.f, 0.79f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container5");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-0.f, 0.f, -14.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container6");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(24.20f, 0.f, -8.f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container7");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(24.2f, 8.f, -22.8f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Container8");
		e->addComponent<ModelComponent>(containerModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(24.36f, 0.f, -32.41f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		e = ECS::Instance()->createEntity("Map_Ramp1");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(5.2f, 0.f, -32.25f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);
		e = ECS::Instance()->createEntity("Map_Ramp2");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(15.2f, 8.f, -32.25f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);
		e = ECS::Instance()->createEntity("Map_Ramp3");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(24.f, 8.f, -5.5f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);
		e = ECS::Instance()->createEntity("Map_Ramp4");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(24.f, 0.f, 9.f), glm::vec3(0.f, 1.57f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);
		e = ECS::Instance()->createEntity("Map_Ramp5");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-16.f, 0.f, 20.f), glm::vec3(0.f, 3.14f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);
		e = ECS::Instance()->createEntity("Map_Ramp6");
		e->addComponent<ModelComponent>(rampModel);
		e->addComponent<StaticMatrixComponent>(glm::vec3(-34.f, 0.f, 20.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addStaticEntity(e);

		// DYNAMIC ENTITIES
		// Use TransformComponent and .addEntity() so that they're interpolated
		e = ECS::Instance()->createEntity("Character");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<PhysicsComponent>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		m_scene.addEntity(e);

		e = ECS::Instance()->createEntity("Character1");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(15.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		m_aiControllers.push_back(e);
		m_scene.addEntity(e);

		e = ECS::Instance()->createEntity("Character2");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 15.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		m_aiControllers.push_back(e);
		m_scene.addEntity(e);

		e = ECS::Instance()->createEntity("Character3");
		e->addComponent<ModelComponent>(characterModel);
		e->addComponent<TransformComponent>(glm::vec3(15.f, 0.f,15.f), glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		e->addComponent<PhysicsComponent>();
		m_aiControllers.push_back(e);
		m_scene.addEntity(e);


		//creates light with model and pointlight
		e = ECS::Instance()->createEntity("Map_Candle1");
		e->addComponent<ModelComponent>(lightModel);
		e->addComponent<TransformComponent>(glm::vec3(3.f, 0.f, 3.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		PointLight pl;
		glm::vec3 lightPos = e->getComponent<TransformComponent>()->getTranslation();
		pl.setColor(glm::vec3(0.2f, 0.2f, 0.2f));
		pl.setPosition(glm::vec3(lightPos.x-0.02f, lightPos.y + 3.1f, lightPos.z));
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		pl.setIndex(0);
		e->addComponent<LightComponent>(pl);
		e->addComponent<LightListComponent>(); // Candle1 holds all lights you can place in debug
		m_scene.addEntity(e);
		m_candles.push_back(e);

		e = ECS::Instance()->createEntity("Map_Candle2");
		e->addComponent<ModelComponent>(lightModel);
		e->addComponent<TransformComponent>(glm::vec3(1.f, 0.f, 1.f));
		e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
		e->addComponent<CollidableComponent>();
		lightPos = e->getComponent<TransformComponent>()->getTranslation();
		pl.setColor(glm::vec3(0.2f, 0.2f, 0.2f));
		pl.setPosition(glm::vec3(lightPos.x - 0.02f, lightPos.y + 3.1f, lightPos.z));
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		pl.setIndex(1);
		e->addComponent<LightComponent>(pl);
		m_scene.addEntity(e);
		m_candles.push_back(e);

		//creates light for the player
		m_playerController.createCandle(lightModel);
		//e = m_playerController.m_candle;//ECS::Instance()->createEntity("PlayerCandle");
		//e->addComponent<ModelComponent>(lightModel);
		//e->addComponent<TransformComponent>(glm::vec3(-1.f, -3.f, 1.f), m_playerController.getEntity()->getComponent<TransformComponent>());
		//e->getComponent<TransformComponent>()->setParent(m_playerController.getEntity()->getComponent<TransformComponent>());

		////lightPos = e->getComponent<TransformComponent>()->getTranslation();
		////pl.setColor(glm::vec3(1.f, 1.f, 1.f));
		////pl.setPosition(glm::vec3(lightPos.x, lightPos.y + 3.1f, lightPos.z));
		////pl.setAttenuation(.0f, 0.1f, 0.02f);
		////pl.setIndex(2);
		////e->addComponent<LightComponent>(pl);
		//m_scene.addEntity(e);


		m_virtRAMHistory = SAIL_NEW float[100];
		m_physRAMHistory = SAIL_NEW float[100];
		// Uncomment this to enable vram budget visualization
		//m_vramBudgetHistory = SAIL_NEW float[100];
		m_vramUsageHistory = SAIL_NEW float[100];
		m_cpuHistory = SAIL_NEW float[100];
		m_frameTimesHistory = SAIL_NEW float[100];
	}
	/* "Unit test" for NodeSystem */
	NodeSystem* test = m_app->getNodeSystem();
#ifdef _DEBUG_NODESYSTEM
	Model* nodeSystemModel = &m_app->getResourceManager().getModel("sphere.fbx", shader);
	nodeSystemModel->getMesh(0)->getMaterial()->setDiffuseTexture("missing.tga");
	test->setDebugModelAndScene(nodeSystemModel, &m_scene);
#endif

	std::vector<NodeSystem::Node> nodes;
	std::vector<std::vector<unsigned int>> connections;

	std::vector<unsigned int> conns;
	int x_max = 60;
	int z_max = 60;
	int x_cur = 0;
	int z_cur = 0;
	int size = x_max * z_max;

	int padding = 2;
	float offsetX = x_max * padding * 0.5f;
	float offsetZ = z_max * padding * 0.5f;
	float offsetY = 0;
	bool* walkable = SAIL_NEW bool[size];

	auto e = ECS::Instance()->createEntity("DeleteMeFirstFrameDummy");
	//e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	//e->addComponent<ModelComponent>(m_boundingBoxModel.get());
	e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
	//m_scene.addEntity(e);


	/*Nodesystem*/
	//ECS::Instance()->update(0.0f); // Update Boundingboxes/octree system here
	ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.f);
	ECS::Instance()->getSystem<OctreeAddRemoverSystem>()->update(0.f);
	for (size_t i = 0; i < size; i++) {
		conns.clear();
		x_cur = i % x_max;
		z_cur = floor(i / x_max);
		glm::vec3 pos(x_cur* padding - offsetX, offsetY, z_cur* padding - offsetZ);
		
		bool blocked = false;
		e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setPosition(pos);
		std::vector < Octree::CollisionInfo> vec;
		m_octree->getCollisions(e.get(), &vec);

		for (Octree::CollisionInfo& info : vec) {
			int i = (info.entity->getName().compare("Map_"));
			if (i >= 0) {
				//Not walkable
				//auto e2 = ECS::Instance()->createEntity("blockedGroundMarker");
				//e2->addComponent<TransformComponent>(pos);
				//e2->addComponent<ModelComponent>(m_boundingBoxModel.get());
				//m_scene.addEntity(e2);

				blocked = true;
				break;
			}
		}

		nodes.emplace_back(pos, blocked, i);

		for (int dx = -1; dx <= 1; dx++) {
			for (int dz = -1; dz <= 1; dz++) {
				if (dx == 0 && dz == 0)
					continue;

				int nx = x_cur + dx;
				int nz = z_cur + dz;
				if (nx >= 0 && nx < x_max && nz >= 0 && nz < z_max) {
					int ni = nx + nz * x_max;
					conns.push_back(ni);
				}
			}
		}

		connections.push_back(conns);
	}
	//Delete "DeleteMeFirstFrameDummy"
	ECS::Instance()->destroyEntity(e);

	test->setNodes(nodes, connections);
	Memory::SafeDeleteArr(walkable);

	m_playerController.provideCandles(&m_candles);
}

GameState::~GameState() {
	delete m_virtRAMHistory;
	delete m_physRAMHistory;
	// Uncomment this to enable vram budget visualization
	//delete m_vramBudgetHistory;
	delete m_vramUsageHistory;
	delete m_cpuHistory;
	delete m_frameTimesHistory;
	delete m_octree;
}

// Process input for the state
// NOTE: Done every frame
bool GameState::processInput(float dt) {

#ifdef _DEBUG
	// Add point light at camera pos by adding it to component
	if (Input::WasKeyJustPressed(SAIL_KEY_E)) {
		PointLight pl;
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(m_cam.getPosition());
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		m_scene.getGameObjectEntityByName("Candle1")->getComponent<LightListComponent>()->m_pls.push_back(pl);
		//m_lights.addPointLight(pl);
	}

#endif
	//Toggle bounding boxes rendering
	if (Input::IsKeyPressed(SAIL_KEY_1)) {
		m_scene.showBoundingBoxes(true);
	}
	if (Input::IsKeyPressed(SAIL_KEY_2)) {
		m_scene.showBoundingBoxes(false);
	}

	if ( Input::WasKeyJustPressed(SAIL_KEY_H) ) {
		for ( int i = 0; i < m_aiControllers.size(); i++ ) {
			if ( m_aiControllers[i].getTargetEntity() == nullptr ) {
				m_aiControllers[i].chaseEntity(m_playerController.getEntity().get());
			} else {
				m_aiControllers[i].chaseEntity(nullptr);
			}
		}
	}

	if (Input::IsKeyPressed(SAIL_KEY_G)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_OEM_5)) {
		m_cc.toggle();
		m_profiler.toggle();
	}

	// Update the camera controller from input devices
	//m_camController.update(dt);
	m_playerController.processMouseInput(dt);
	for ( auto& ai : m_aiControllers ) {
		ai.update(dt);
	}
	//m_physSystem.execute(dt);


	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadShader<MaterialShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
	}

	//checks if candle entity has light and if not, adds one 
	if (Input::WasKeyJustPressed(SAIL_KEY_Z)) {
		if (!m_scene.getGameObjectEntityByName("Candle1")->hasComponent<LightComponent>()) {
			PointLight pl;
			glm::vec3 pos = m_scene.getGameObjectEntityByName("Candle1")->getComponent<TransformComponent>()->getTranslation();
			pl.setColor(glm::vec3(1.f, 1.f, 1.f));
			pl.setPosition(glm::vec3(pos.x, pos.y + 3.1, pos.z));
			pl.setAttenuation(.0f, 0.1f, 0.02f);
			pl.setIndex(0);
			m_scene.getGameObjectEntityByName("Candle1")->addComponent<LightComponent>(pl);
			//m_lights.addPointLight(pl);
		}
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_V)) {
		if (!m_scene.getGameObjectEntityByName("Candle2")->hasComponent<LightComponent>()) {
			PointLight pl;
			glm::vec3 pos = m_scene.getGameObjectEntityByName("Candle2")->getComponent<TransformComponent>()->getTranslation();
			pl.setColor(glm::vec3(1.f, 1.f, 1.f));
			pl.setPosition(glm::vec3(pos.x, pos.y + 3.1, pos.z));
			pl.setAttenuation(.0f, 0.1f, 0.02f);
			pl.setIndex(1);
			m_scene.getGameObjectEntityByName("Candle2")->addComponent<LightComponent>(pl);
			//m_lights.addPointLight(pl);
		}
	}

	//removes first added pointlight in arena
	if (Input::WasKeyJustPressed(SAIL_KEY_X)) {

		if (m_scene.getGameObjectEntityByName("Candle1")->getComponent<LightListComponent>()->m_pls.size() > 0) {
			m_scene.getGameObjectEntityByName("Candle1")->getComponent<LightListComponent>()->m_pls.erase(m_scene.getGameObjectEntityByName("Candle1")->getComponent<LightListComponent>()->m_pls.begin());
		}

		//m_lights.removePointLight();
	}
	return true;
	}


bool GameState::onEvent(Event& event) {
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&GameState::onResize));

	// Forward events
	m_scene.onEvent(event);

	return true;
}

bool GameState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool GameState::update(float dt) {
	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | " + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1.0f;
	static float change = 0.4f;
	
	counter += dt * 2.0f;

	// TODO: make a system or something for this
	m_playerController.destroyOldProjectiles();

	m_playerController.update(dt);

	m_scene.prepareUpdate(); // Copy game state from previous tick
	m_playerController.prepareUpdate(); // Copy player position from previous tick

	m_playerController.processKeyboardInput(TIMESTEP);


	m_lights.clearPointLights();
	updateComponentSystems(dt);

	//check and update all lights for all entities

	// MOVED TO LIGHT SYSTEM
	std::vector<Entity::SPtr> entities = m_scene.getGameObjectEntities();
	m_lights.addPointLight(m_playerController.getCandle()->getComponent<LightComponent>()->m_pointLight);
	for (int i = 0; i < entities.size();i++) {
		if (entities[i]->hasComponent<LightComponent>()) {
			m_lights.addPointLight(entities[i]->getComponent<LightComponent>()->m_pointLight);
		}
		if (entities[i]->hasComponent<LightListComponent>()) {
			for (int j = 0; j < entities[i]->getComponent<LightListComponent>()->m_pls.size(); j++) {
				m_lights.addPointLight(entities[i]->getComponent<LightListComponent>()->m_pls[j]);
			}
		}
	}
	/*m_lights.updateBufferData();*/

	// copy per-frame render objects to their own list so that they can be rendered without
	// any interference from the update loop
	m_scene.prepareRenderObjects();

	return true;
}

// Renders the state
// DO NOT CREATE OR DESTROY ANY gameObjects HERE
// alpha is a the interpolation value (range [0,1]) between the last two snapshots
bool GameState::render(float dt, float alpha) {
	// Interpolate the player's camera position (but not rotation)
	m_playerController.updateCameraPosition(alpha);

	m_lights.updateBufferData();


	// Clear back buffer
	m_app->getAPI()->clear({ 0.01f, 0.01f, 0.01f, 1.0f });

	// Draw the scene
	m_scene.draw(m_cam, alpha);

	return true;
}

bool GameState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	renderImguiConsole(dt);
	renderImguiProfiler(dt);
	renderImGuiRenderSettings(dt);
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

			//std::string* str = new std::string(m_cc.getTextField());
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

			// Uncomment this to enable vram budget visualization

			/*header = "VRAM Available (" + m_vramBCount + " MB)";
			ImGui::Text(header.c_str());*/


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

			// Uncomment this to enable vram budget visualization

			/*if (ImGui::CollapsingHeader("VRAM Budget Graph")) {
				header = "\n\n\n" + m_vramBCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_vramBudgetHistory, 100, 0, "", 0.f, 6000.f, ImVec2(0, 100));
			}*/



			ImGui::EndChild();

			m_profilerTimer += dt;
			if (m_profilerTimer > 0.2f) {
				m_profilerTimer = 0.f;
				if (m_profilerCounter < 100) {

					// Uncomment this to enable vram budget visualization

					//m_vramBudgetHistory[m_profilerCounter] = m_profiler.vramBudget();
					//m_vramBCount = "\n\n\n" + std::to_string(m_profiler.vramBudget());

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

					// Uncomment this to enable vram budget visualization

					/*float* tempFloatArr2 = SAIL_NEW float[100];
					std::copy(m_vramBudgetHistory + 1, m_vramBudgetHistory + 101, tempFloatArr2);
					tempFloatArr2[99] = m_profiler.vramBudget();
					delete m_vramBudgetHistory;
					m_vramBudgetHistory = tempFloatArr2;
					m_vramBCount = std::to_string(m_profiler.vramBudget());*/

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
	const char* items[] = { "Forward raster", "Raytraced" };
	static int selectedRenderer = 0;
	if (ImGui::Combo("Renderer", &selectedRenderer, items, IM_ARRAYSIZE(items))) {
		m_scene.changeRenderer(selectedRenderer);
	}
	ImGui::Checkbox("Enable post processing", &m_scene.getDoProcessing());
	ImGui::End();

	return false;
}

void GameState::updateComponentSystems(float dt) {
	m_componentSystems.updateBoundingBoxSystem->update(dt);
	m_componentSystems.octreeAddRemoverSystem->update(dt);
	m_componentSystems.physicSystem->update(dt);
	m_componentSystems.animationSystem->update(dt);
}

const std::string GameState::createCube(const glm::vec3& position) {
	auto e = ECS::Instance()->createEntity("new cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(position);
	e->addComponent<BoundingBoxComponent>(m_boundingBoxModel.get());
	e->addComponent<CollidableComponent>();
	m_scene.addEntity(e);
	return std::string("Added Cube at (" +
		std::to_string(position.x) + ":" +
		std::to_string(position.y) + ":" +
		std::to_string(position.z) + ")");
}