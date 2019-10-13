#include "PerformanceTestState.h"
#include "imgui.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/systems/candles/CandleSystem.h"
#include "Sail/entities/systems/entityManagement/EntityRemovalSystem.h"
#include "Sail/entities/systems/entityManagement/EntityAdderSystem.h"
#include "Sail/entities/systems/lifetime/LifeTimeSystem.h"
#include "Sail/entities/systems/light/LightSystem.h"
#include "Sail/entities/systems/gameplay/GunSystem.h"
#include "Sail/entities/systems/Gameplay/ProjectileSystem.h"
#include "Sail/entities/systems/Graphics/AnimationSystem.h"
#include "Sail/entities/systems/physics/OctreeAddRemoverSystem.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "Sail/entities/systems/prepareUpdate/PrepareUpdateSystem.h"
#include "Sail/entities/systems/Input/GameInputSystem.h"
#include "Sail/entities/systems/Audio/AudioSystem.h"
#include "Sail/entities/systems/render/RenderSystem.h"
#include "Sail/TimeSettings.h"

#include <sstream>
#include <iomanip>

// Uncomment to use forward rendering
//#define DISABLE_RT

PerformanceTestState::PerformanceTestState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
	, m_camController(&m_cam)
	, m_profiler(true) {
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
#ifdef _DEBUG
	console.addCommand(std::string("AddCube"), [&] () {
		return createCube(m_cam.getPosition());
	}, "PerfTestState");
	console.addCommand(std::string("AddCube <int> <int> <int>"), [&] (std::vector<int> in) {
		if ( in.size() == 3 ) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "PerfTestState");
	console.addCommand(std::string("AddCube <float> <float> <float>"), [&] (std::vector<float> in) {
		if ( in.size() == 3 ) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "PerfTestState");
#endif

	// Create octree
	m_octree = SAIL_NEW Octree(nullptr);

	m_renderSettingsWindow.activateMaterialPicking(&m_cam, m_octree);

	// Get the Application instance
	m_app = Application::getInstance();
	m_componentSystems.renderSystem = ECS::Instance()->getSystem<RenderSystem>();

	// Create entity adder system
	m_componentSystems.entityAdderSystem = ECS::Instance()->getEntityAdderSystem();

	// Create entity removal system
	m_componentSystems.entityRemovalSystem = ECS::Instance()->getEntityRemovalSystem();

	// Create system for updating bounding box
	m_componentSystems.updateBoundingBoxSystem = ECS::Instance()->createSystem<UpdateBoundingBoxSystem>();

	// Create system for handling octree
	m_componentSystems.octreeAddRemoverSystem = ECS::Instance()->createSystem<OctreeAddRemoverSystem>();
	m_componentSystems.octreeAddRemoverSystem->provideOctree(m_octree);


	// Textures needs to be loaded before they can be used
	Application::getInstance()->getResourceManager().loadTexture("pbr/pavingStones/albedo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/pavingStones/metalnessRoughnessAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/pavingStones/normal.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/greenTiles/albedo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/greenTiles/metalnessRoughnessAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/greenTiles/normal.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/albedo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/metalnessRoughnessAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/metal/normal.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/ice/albedo.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/ice/metalnessRoughnessAO.tga");
	Application::getInstance()->getResourceManager().loadTexture("pbr/ice/normal.tga");

	Application::getInstance()->getResourceManager().loadTexture("pbr/brdfLUT.tga");


	// Add a directional light
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));

	PointLight pl;
	pl.setColor(glm::vec3(1.0f));
	pl.setPosition(glm::vec3(0.f, 10.f, 0.f));
	m_lights.addPointLight(pl);

	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);
	// Disable culling for testing purposes
	m_app->getAPI()->setFaceCulling(GraphicsAPI::NO_CULLING);

#ifdef DISABLE_RT
	auto* shader = &m_app->getResourceManager().getShaderSet<MaterialShader>();
	( *Application::getInstance()->getRenderWrapper() ).changeRenderer(1);
	m_componentSystems.renderSystem->refreshRenderer();
	m_app->getRenderWrapper()->getCurrentRenderer()->setLightSetup(&m_lights);
#else
	auto* shader = &m_app->getResourceManager().getShaderSet<GBufferOutShader>();
#endif

	// Create/load models

	m_cubeModel = ModelFactory::CubeModel::Create(glm::vec3(0.5f), shader);
	m_cubeModel->getMesh(0)->getMaterial()->setColor(glm::vec4(0.2f, 0.8f, 0.4f, 1.0f));

	m_planeModel = ModelFactory::PlaneModel::Create(glm::vec2(20.f, 20.f), shader, glm::vec2(4.0f));
	m_planeModel->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/ice/albedo.tga");
	m_planeModel->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/ice/metalnessRoughnessAO.tga");
	m_planeModel->getMesh(0)->getMaterial()->setNormalTexture("pbr/ice/normal.tga");

	Model* cylinderModel0 = &m_app->getResourceManager().getModel("pbrCylinder.fbx", shader);
	cylinderModel0->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/metal/albedo.tga");
	cylinderModel0->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/metal/metalnessRoughnessAO.tga");
	cylinderModel0->getMesh(0)->getMaterial()->setNormalTexture("pbr/metal/normal.tga");

	Model* cylinderModel1 = &m_app->getResourceManager().getModel("pbrCylinder_.fbx", shader);
	cylinderModel1->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/pavingStones/albedo.tga");
	cylinderModel1->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/pavingStones/metalnessRoughnessAO.tga");
	cylinderModel1->getMesh(0)->getMaterial()->setNormalTexture("pbr/pavingStones/normal.tga");

	Model* cylinderModel2 = &m_app->getResourceManager().getModel("pbrCylinder__.fbx", shader);
	cylinderModel2->getMesh(0)->getMaterial()->setAlbedoTexture("pbr/greenTiles/albedo.tga");
	cylinderModel2->getMesh(0)->getMaterial()->setMetalnessRoughnessAOTexture("pbr/greenTiles/metalnessRoughnessAO.tga");
	cylinderModel2->getMesh(0)->getMaterial()->setNormalTexture("pbr/greenTiles/normal.tga");

	/*
		Creation of entities
	*/

	{
		auto e = ECS::Instance()->createEntity("Plane");
		e->addComponent<ModelComponent>(m_planeModel.get());
		e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
		e->addComponent<BoundingBoxComponent>();
		e->addComponent<CollidableComponent>();

		e = ECS::Instance()->createEntity("Cylinder1");
		e->addComponent<ModelComponent>(cylinderModel0);
		e->addComponent<TransformComponent>(glm::vec3(0.f, 1.f, 0.f));
		e->addComponent<BoundingBoxComponent>();
		e->addComponent<CollidableComponent>();

		e = ECS::Instance()->createEntity("Cylinder2");
		e->addComponent<ModelComponent>(cylinderModel1);
		e->addComponent<TransformComponent>(glm::vec3(3.f, 1.f, 0.f));
		e->addComponent<BoundingBoxComponent>();
		e->addComponent<CollidableComponent>();

		e = ECS::Instance()->createEntity("Cylinder3");
		e->addComponent<ModelComponent>(cylinderModel2);
		e->addComponent<TransformComponent>(glm::vec3(-3.f, 1.f, 0.f));
		e->addComponent<BoundingBoxComponent>();
		e->addComponent<CollidableComponent>();
	}

}

PerformanceTestState::~PerformanceTestState() {
	delete m_octree;
}

// Process input for the state
// NOTE: Done every frame
bool PerformanceTestState::processInput(float dt) {

	//#ifdef _DEBUG
		// Add point light at camera pos
	if ( Input::WasKeyJustPressed(KeyBinds::addLight) ) {
		PointLight pl;
		pl.setPosition(m_cam.getPosition());
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		m_lights.addPointLight(pl);
	}

	//#endif

	if ( Input::IsKeyPressed(KeyBinds::setDirectionalLight) ) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Reload shaders
	if ( Input::WasKeyJustPressed(KeyBinds::reloadShader) ) {
		m_app->getResourceManager().reloadShader<GBufferOutShader>();
	}

	m_camController.update(dt);

	return false;
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

	m_lights.updateBufferData();
	return false;
}

bool PerformanceTestState::fixedUpdate(float dt) {

	m_componentSystems.entityAdderSystem->update(0.0f);
	m_componentSystems.updateBoundingBoxSystem->update(dt);
	m_componentSystems.octreeAddRemoverSystem->update(dt);

	std::wstring fpsStr = std::to_wstring(m_app->getFPS());

	m_app->getWindow()->setWindowTitle("Sail | Game Engine Demo | "
									   + Application::getPlatformName() + " | FPS: " + std::to_string(m_app->getFPS()));

	static float counter = 0.0f;
	static float size = 1.0f;
	static float change = 0.4f;

	counter += dt * 2.0f;

	// Will probably need to be called last
	m_componentSystems.entityRemovalSystem->update(0.0f);

	return false;
}

// Renders the state
// alpha is a the interpolation value (range [0,1]) between the last two snapshots
bool PerformanceTestState::render(float dt, float alpha) {
	// Clear back buffer
	m_app->getAPI()->clear({ 0.01f, 0.01f, 0.01f, 1.0f });

	// Draw the scene. Entities with model and trans component will be rendered.
	m_componentSystems.renderSystem->draw(m_cam, alpha);

	return false;
}

bool PerformanceTestState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	renderImGuiLightDebug(dt);
	m_profiler.renderWindow();
	m_renderSettingsWindow.renderWindow();
	return false;
}

bool PerformanceTestState::renderImGuiLightDebug(float dt) {
	ImGui::Begin("Light debug");
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

const std::string PerformanceTestState::createCube(const glm::vec3& position) {
	auto e = ECS::Instance()->createEntity("new cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(position);
	return std::string("Added Cube at (" +
					   std::to_string(position.x) + ":" +
					   std::to_string(position.y) + ":" +
					   std::to_string(position.z) + ")");
}