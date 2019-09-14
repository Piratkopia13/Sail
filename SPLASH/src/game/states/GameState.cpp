#include "GameState.h"
#include "imgui.h"

GameState::GameState(StateStack& stack)
: State(stack)
//, m_cam(20.f, 20.f, 0.1f, 5000.f)
, m_cam(90.f, 1280.f / 720.f, 0.1f, 5000.f)
, m_camController(&m_cam)
{

	// Get the Application instance
	m_app = Application::getInstance();
	//m_scene = std::make_unique<Scene>(AABB(glm::vec3(-100.f, -100.f, -100.f), glm::vec3(100.f, 100.f, 100.f)));

	// Textures needs to be loaded before they can be used
	// TODO: automatically load textures when needed so the following can be removed
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_ddn.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_diff.tga");
	Application::getInstance()->getResourceManager().loadTexture("sponza/textures/spnza_bricks_a_spec.tga");

	// Set up camera with controllers
	m_cam.setPosition(glm::vec3(1.6f, 4.7f, 7.4f));
	m_camController.lookAt(glm::vec3(0.f));
	
	// Add a directional light
	glm::vec3 color(1.0f, 1.0f, 1.0f);
 	glm::vec3 direction(0.4f, -0.2f, 1.0f);
	direction = glm::normalize(direction);
	m_lights.setDirectionalLight(DirectionalLight(color, direction));
	// Add four point lights
	{
		PointLight pl;
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(-4.0f, 0.1f, -4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(-4.0f, 0.1f, 4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(4.0f, 0.1f, 4.0f));
		m_lights.addPointLight(pl);

		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(glm::vec3(4.0f, 0.1f, -4.0f));
		m_lights.addPointLight(pl);
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

	// Create entities
	auto e = Entity::Create("Static cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(-4.f, 1.f, -2.f));
	m_scene.addEntity(e);

	e = Entity::Create("Floor");
	e->addComponent<ModelComponent>(m_planeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	m_scene.addEntity(e);

	e = Entity::Create("Clingy cube");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(-1.2f, 1.f, -1.f), glm::vec3(0.f, 0.f, 1.07f));
	m_scene.addEntity(e);

	// Add some cubes which are connected through parenting
	m_texturedCubeEntity = Entity::Create("Textured parent cube");
	m_texturedCubeEntity->addComponent<ModelComponent>(fbxModel);
	m_texturedCubeEntity->addComponent<TransformComponent>(glm::vec3(-1.f, 2.f, 0.f), m_texturedCubeEntity->getComponent<TransformComponent>());
	m_texturedCubeEntity->setName("MovingCube");
	m_scene.addEntity(m_texturedCubeEntity);
	e->getComponent<TransformComponent>()->setParent(m_texturedCubeEntity->getComponent<TransformComponent>());

	e = Entity::Create("CubeRoot");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(10.f, 0.f, 10.f));
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("CubeChild");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[0]->getComponent<TransformComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

	e = Entity::Create("CubeChildChild");
	e->addComponent<ModelComponent>(m_cubeModel.get());
	e->addComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f), m_transformTestEntities[1]->getComponent<TransformComponent>());
	m_scene.addEntity(e);
	m_transformTestEntities.push_back(e);

}

GameState::~GameState() {
}

// Process input for the state
bool GameState::processInput(float dt) {

#ifdef _DEBUG
	// Add point light at camera pos
	if (Input::WasKeyJustPressed(SAIL_KEY_E)) {
		PointLight pl;
		pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
		pl.setPosition(m_cam.getPosition());
		pl.setAttenuation(.0f, 0.1f, 0.02f);
		m_lights.addPointLight(pl);
	}

	if (Input::WasKeyJustPressed(SAIL_KEY_1)) {
		Logger::Log("Setting parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->setParent(m_transformTestEntities[1]->getComponent<TransformComponent>());
	}
	if (Input::WasKeyJustPressed(SAIL_KEY_2)) {
		Logger::Log("Removing parent");
		m_transformTestEntities[2]->getComponent<TransformComponent>()->removeParent();
	}
#endif

	if (Input::IsKeyPressed(SAIL_KEY_G)) {
		glm::vec3 color(1.0f, 1.0f, 1.0f);;
		m_lights.setDirectionalLight(DirectionalLight(color, m_cam.getDirection()));
	}

	// Update the camera controller from input devices
	m_camController.update(dt);

	// Reload shaders
	if (Input::WasKeyJustPressed(SAIL_KEY_R)) {
		m_app->getResourceManager().reloadShader<MaterialShader>();
		Event e(Event::POTATO);
		m_app->dispatchEvent(e);
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
	static float size = 1;
	static float change = 0.4f;

	counter += dt * 2;
	if (m_texturedCubeEntity) {
		// Move the cubes around
		m_texturedCubeEntity->getComponent<TransformComponent>()->setTranslation(glm::vec3(glm::sin(counter), 1.f, glm::cos(counter)));
		m_texturedCubeEntity->getComponent<TransformComponent>()->setRotations(glm::vec3(glm::sin(counter), counter, glm::cos(counter)));
	}

	if (m_transformTestEntities.size() > 0) {
		// Move the three parented cubes with identical translation, rotations and scale to show how parenting affects transforms
		for (Entity::SPtr item : m_transformTestEntities) {
			item->getComponent<TransformComponent>()->rotateAroundY(dt * 1.0f);
			item->getComponent<TransformComponent>()->setScale(size);
			item->getComponent<TransformComponent>()->setTranslation(size * 3, 1.0f, size * 3);
		}
		m_transformTestEntities[0]->getComponent<TransformComponent>()->translate(2.0f, 0.0f, 2.0f);
	}

	size += change * dt;
	if (size > 1.2f || size < 0.7f)
		change *= -1.0f;

	return true;
}

// Renders the state
bool GameState::render(float dt) {

	// Clear back buffer
	m_app->getAPI()->clear({0.1f, 0.2f, 0.3f, 1.0f});

	// Draw the scene
	m_scene.draw(m_cam);

	return true;
}

bool GameState::renderImgui(float dt) {
	// The ImGui window is rendered when activated on F10
	ImGui::ShowDemoWindow();

	ImGui::Begin("Rendering settings");
	const char* items[] = { "Forward raster", "Raytraced" };
	static int selectedRenderer = 0;
	if (ImGui::Combo("Renderer", &selectedRenderer, items, IM_ARRAYSIZE(items))) {
		m_scene.changeRenderer(selectedRenderer);
	}
	ImGui::End();

	ImGui::Begin("Performance");
	ImGui::Text("VRAM usage: %u / %u MB", m_app->getAPI()->getMemoryUsage(), m_app->getAPI()->getMemoryBudget());
	ImGui::End();


	ImGui::Begin("Post processing");
	// Reordering is actually a rather odd use case for the drag and drop API which is meant to carry data around. 
	// Here we implement a little demo using the drag and drop primitives, but we could perfectly achieve the same results by using a mixture of
	//  IsItemActive() on the source item + IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) on target items.
	// This demo however serves as a pretext to demonstrate some of the flags you can use with BeginDragDropSource() and AcceptDragDropPayload().
	ImGui::BulletText("Drag and drop to re-order");

	ImGui::Columns(3, NULL);
	ImGui::Separator();
	ImGui::Text("Stage"); ImGui::NextColumn();
	ImGui::Text("Resolution scale"); ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::Separator();
	
	static const char* usedStages[] = { "Gaussian blur horizontal", "Gaussian blur vertical", "Tonemapping", "Bloom", "FXAA" };
	int move_from = -1, move_to = -1;
	static int selected = -1;
	for (int n = 0; n < IM_ARRAYSIZE(usedStages); n++) {
		if (ImGui::Selectable(usedStages[n], selected == n, 0, ImVec2(0, 19))) {
			selected = n;
		}

		ImGuiDragDropFlags src_flags = 0;
		src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
		src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging
		//src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
		if (ImGui::BeginDragDropSource(src_flags)) {
			if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
				ImGui::Text("Moving \"%s\"", usedStages[n]);
			ImGui::SetDragDropPayload("DND_DEMO_NAME", &n, sizeof(int));
			ImGui::EndDragDropSource();
		}
		
		if (ImGui::BeginDragDropTarget()) {
			ImGuiDragDropFlags target_flags = 0;
			target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
			target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle
			if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("DND_DEMO_NAME", target_flags)) {
				move_from = *(const int*)payload->Data;
				move_to = n;
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::NextColumn();
	static float resScale[IM_ARRAYSIZE(usedStages)];
	for (int n = 0; n < IM_ARRAYSIZE(usedStages); n++) {
		ImGui::SliderFloat((std::string("##resScale")+std::to_string(n)).c_str(), &resScale[n], 0.f, 1.0f);
	}
	ImGui::NextColumn();
	for (int n = 0; n < IM_ARRAYSIZE(usedStages); n++) {
		ImGui::Button("Remove");
	}

	if (move_from != -1 && move_to != -1) {
		// Reorder items
		int copy_dst = (move_from < move_to) ? move_from : move_to + 1;
		int copy_src = (move_from < move_to) ? move_from + 1 : move_to;
		int copy_count = (move_from < move_to) ? move_to - move_from : move_from - move_to;
		const char* tmp = usedStages[move_from];
		float tmp2 = resScale[move_from];
		memmove(&usedStages[copy_dst], &usedStages[copy_src], (size_t)copy_count * sizeof(const char*));
		memmove(&resScale[copy_dst], &resScale[copy_src], (size_t)copy_count * sizeof(float));
		usedStages[move_to] = tmp;
		resScale[move_to] = tmp2;
		ImGui::SetDragDropPayload("DND_DEMO_NAME", &move_to, sizeof(int)); // Update payload immediately so on the next frame if we move the mouse to an earlier item our index payload will be correct. This is odd and showcase how the DnD api isn't best presented in this example.
	}
	ImGui::Separator();
	ImGui::Columns(1);
	ImGui::Spacing();

	// Add new stage dropdown list
	static int currentItem = 0;
	static const char* availableStages[] = { "Gaussian blur horizontal", "Gaussian blur vertical", "Tonemapping", "Bloom", "FXAA" };
	ImGui::Combo("##newStage", &currentItem, availableStages, IM_ARRAYSIZE(availableStages));
	ImGui::SameLine();
	ImGui::Button("Add");

	ImGui::End();

	return false;
}
