#include "EditorGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

EditorGui::EditorGui() {
	m_modelName = "None - click to load";
}

void EditorGui::render(float dt, FUNC(void(CallbackType type, const std::string&)) callback, Entity* entity) {
	newFrame();
	m_callback = callback;

	float menuBarHeight = setupMenuBar();

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		setupDockspace(menuBarHeight);
	}

	// Environment settings window
	ImGui::Begin("Environment");

	enableColumns(90.f);
	addProperty("Environment", [&]() {
		const char* environments[] = { "studio", "rail", "pier" };
		static int currentEnvironmentIndex = 0;
		ImGui::Combo("##hideLabel", &currentEnvironmentIndex, environments, IM_ARRAYSIZE(environments));
		m_callback(CallbackType::ENVIRONMENT_CHANGED, environments[currentEnvironmentIndex]);
	});
	disableColumns();

	ImGui::End();
	//ImGui::Separator();
	//enableColumns();
	//LimitStringLength(m_modelName);
	//addProperty("Model", [&]() {
	//	if (ImGui::Button(m_modelName.c_str(), ImVec2(propWidth, 0))) {
	//		std::string newModel = OpenFileDialog(L"FBX models (*.fbx)\0*.fbx");
	//		if (!newModel.empty()) {
	//			m_modelName = newModel;
	//			m_callback(CallbackType::MODEL_CHANGED, m_modelName);
	//		}
	//	}
	//});
	//	
	//// ================================
	////			 MATERIAL GUI
	//// ================================
	//MaterialComponent* materialComp = nullptr;
	//ModelComponent* modelComp = entity->getComponent<ModelComponent>();
	//if (modelComp) {
	//	if (!(materialComp = entity->getComponent<MaterialComponent>())) {
	//		disableColumns();
	//		ImGui::Separator();
	//		ImGui::Text("Model is missing material component");
	//		ImGui::Separator();
	//		enableColumns();
	//	}
	//}
	//if (materialComp) {
	//	disableColumns();
	//	ImGui::Separator();
	//	ImGui::Text("Shader pipeline: %s", modelComp->getModel()->getMesh(0)->getShader()->getPipeline()->getName().c_str()); ImGui::NextColumn();
	//	ImGui::Separator();
	//	enableColumns();

	//	materialComp->renderEditorGui(this);
	//}

	//// ================================
	////			TRANSFORM GUI
	//// ================================
	//TransformComponent* transformComp = entity->getComponent<TransformComponent>();
	//if (modelComp && transformComp) {
	//	disableColumns();
	//	ImGui::Separator();
	//	ImGui::Text("Transform");
	//	enableColumns(90.f);

	//	transformComp->renderEditorGui(this);
	//}

	//disableColumns();
}

void EditorGui::setupDockspace(float menuBarHeight) {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 windowPos = viewport->Pos;
	windowPos.y += menuBarHeight;
	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
	host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	host_window_flags |= ImGuiWindowFlags_NoBackground;

	char label[32];
	ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(label, NULL, host_window_flags);
	ImGui::PopStyleVar(3);
	
	ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}

float EditorGui::setupMenuBar(){
	static bool showDemoWindow;
	ImVec2 menuBarSize;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Duffle")) {
			ImGui::MenuItem("Show demo window", NULL, &showDemoWindow);
			if (ImGui::MenuItem("Switch to GameState", NULL)) { m_callback(CallbackType::CHANGE_STATE, ""); }
			if (ImGui::MenuItem("Exit", NULL)) { PostQuitMessage(0); }
			ImGui::EndMenu();
		}

		menuBarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	if (showDemoWindow) {
		ImGui::ShowDemoWindow();
	}
	return menuBarSize.y;
}

