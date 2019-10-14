#include "PBRTestState.h"
#include "imgui.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/Systems.h"
#include "Sail/TimeSettings.h"

#include <sstream>
#include <iomanip>

// Uncomment to use forward rendering
//#define DISABLE_RT

PBRTestState::PBRTestState(StateStack& stack)
	: State(stack)
	, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
	, m_camController(&m_cam)
	, m_profiler(true) {
	auto& console = Application::getInstance()->getConsole();
	console.addCommand("state <string>", [&](const std::string& param) {
		if (param == "menu") {
			requestStackPop();
			requestStackPush(States::MainMenu);
			console.removeAllCommandsWithIdentifier("PBRTestState");
			return "State change to menu requested";
		} else {
			return "Invalid state. Available states are \"menu\"";
		}
	}, "PBRTestState");
#ifdef _DEBUG
	console.addCommand(std::string("AddCube"), [&]() {
		return createCube(m_cam.getPosition());
	}, "PBRTestState");
	console.addCommand(std::string("AddCube <int> <int> <int>"), [&](std::vector<int> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "PBRTestState");
	console.addCommand(std::string("AddCube <float> <float> <float>"), [&](std::vector<float> in) {
		if (in.size() == 3) {
			glm::vec3 pos(in[0], in[1], in[2]);
			return createCube(pos);
		} else {
			return std::string("Error: wrong number of inputs. Console Broken");
		}
		return std::string("wat");
	}, "PBRTestState");
#endif

	// Create octree
	m_octree = SAIL_NEW Octree(nullptr);

	// Get the Application instance
	m_app = Application::getInstance();
	
	// Create systems for rendering
	m_componentSystems.beginEndFrameSystem = ECS::Instance()->createSystem<BeginEndFrameSystem>();
	m_componentSystems.boundingboxSubmitSystem = ECS::Instance()->createSystem<BoundingboxSubmitSystem>();
	m_componentSystems.metaballSubmitSystem = ECS::Instance()->createSystem<MetaballSubmitSystem>();
	m_componentSystems.modelSubmitSystem = ECS::Instance()->createSystem<ModelSubmitSystem>();
	m_componentSystems.realTimeModelSubmitSystem = ECS::Instance()->createSystem<RealTimeModelSubmitSystem>();

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
	(*Application::getInstance()->getRenderWrapper()).changeRenderer(1);
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
		auto e = EntityFactory::CreateStaticMapObject("Plane", m_planeModel.get(), nullptr, glm::vec3(0.f, 0.f, 0.f));
		e = EntityFactory::CreateStaticMapObject("Cylinder1", cylinderModel0, nullptr, glm::vec3(0.f, 1.f, 0.f));
		e = EntityFactory::CreateStaticMapObject("Cylinder2", cylinderModel1, nullptr, glm::vec3(3.f, 1.f, 0.f));
		e = EntityFactory::CreateStaticMapObject("Cylinder3", cylinderModel2, nullptr, glm::vec3(-3.f, 1.f, 0.f));

		m_virtRAMHistory = SAIL_NEW float[100];
		m_physRAMHistory = SAIL_NEW float[100];
		m_vramUsageHistory = SAIL_NEW float[100];
		m_cpuHistory = SAIL_NEW float[100];
		m_frameTimesHistory = SAIL_NEW float[100];
	}

}

PBRTestState::~PBRTestState() {
	delete m_virtRAMHistory;
	delete m_physRAMHistory;
	delete m_vramUsageHistory;
	delete m_cpuHistory;
	delete m_frameTimesHistory;
	delete m_octree;
}

// Process input for the state
// NOTE: Done every frame
bool PBRTestState::processInput(float dt) {

//#ifdef _DEBUG
	// Add point light at camera pos
	if (Input::WasKeyJustPressed(KeyBinds::addLight)) {
		PointLight pl;
		pl.setPosition(m_cam.getPosition());
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		m_lights.addPointLight(pl);
	}

//#endif

	if (Input::IsKeyPressed(KeyBinds::setDirectionalLight)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Reload shaders
	if (Input::WasKeyJustPressed(KeyBinds::reloadShader)) {
		m_app->getResourceManager().reloadShader<GBufferOutShader>();
	}

	m_camController.update(dt);

	return false;
}


bool PBRTestState::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&PBRTestState::onResize));

	return true;
}

bool PBRTestState::onResize(WindowResizeEvent& event) {
	m_cam.resize(event.getWidth(), event.getHeight());
	return true;
}

bool PBRTestState::update(float dt, float alpha) {

	m_lights.updateBufferData();
	return false;
}

bool PBRTestState::fixedUpdate(float dt) {

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
	m_componentSystems.entityAdderSystem->update();
	m_componentSystems.entityRemovalSystem->update();

	return false;
}

// Renders the state
// alpha is a the interpolation value (range [0,1]) between the last two snapshots
bool PBRTestState::render(float dt, float alpha) {
	// Clear back buffer
	m_app->getAPI()->clear({ 0.01f, 0.01f, 0.01f, 1.0f });

	// Draw the scene. Entities with model and trans component will be rendered.
	m_componentSystems.beginEndFrameSystem->beginFrame(m_cam);

	m_componentSystems.modelSubmitSystem->submitAll(alpha);
	m_componentSystems.realTimeModelSubmitSystem->submitAll(alpha);
	m_componentSystems.metaballSubmitSystem->submitAll(alpha);
	m_componentSystems.boundingboxSubmitSystem->submitAll();

	m_componentSystems.beginEndFrameSystem->endFrameAndPresent();

	return false;
}

bool PBRTestState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();
	renderImguiProfiler(dt);
	renderImGuiRenderSettings(dt);
	renderImGuiLightDebug(dt);

	return false;
}

bool PBRTestState::renderImguiProfiler(float dt) {
	bool open = m_profiler.isWindowOpen();
	if (open) {
		if (ImGui::Begin("Profiler", &open)) {
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

bool PBRTestState::renderImGuiRenderSettings(float dt) {
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
		ImGui::Text("Material properties for %s", pickedEntity->getName());
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

bool PBRTestState::renderImGuiLightDebug(float dt) {
	ImGui::Begin("Light debug");
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

const std::string PBRTestState::createCube(const glm::vec3& position) {
	auto e = ECS::Instance()->createEntity("new cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(position);
	return std::string("Added Cube at (" +
		std::to_string(position.x) + ":" +
		std::to_string(position.y) + ":" +
		std::to_string(position.z) + ")");
}