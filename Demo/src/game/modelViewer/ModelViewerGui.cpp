#include "ModelViewerGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

ModelViewerGui::ModelViewerGui() 
	: m_propID(0)
	, m_propWidth(0)
	, m_setNextPropWidth(true)
{
	m_modelName = "None - click to load";
}

void ModelViewerGui::render(float dt, FUNC(void()) funcSwitchState, FUNC(void(const std::string&)) callbackNewModel, Entity* entity) {
	m_funcSwitchState = funcSwitchState;
	m_callbackNewModel = callbackNewModel;

	// Reset propID for imgui to function properly
	m_propID = 0;

	float menuBarHeight = setupMenuBar();

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		setupDockspace(menuBarHeight);
	}

	// Model customization window
	ImGui::Begin("Model customizer");

	enableColumns();

	limitStringLength(m_modelName);
	addProperty("Model", [&]() {
		if (ImGui::Button(m_modelName.c_str(), ImVec2(m_propWidth, 0))) {
			std::string newModel = openFileDialog(L"FBX models (*.fbx)\0*.fbx");
			if (!newModel.empty()) {
				m_modelName = newModel;
				m_callbackNewModel(m_modelName);
			}
		}
	});
		
	static const char* lbl = "##hidelabel";
	
	// ================================
	//			 MATERIAL GUI
	// ================================
	Material* material = nullptr;
	ModelComponent* model = entity->getComponent<ModelComponent>();
	if (model) {
		material = model->getModel()->getMesh(0)->getMaterial();
	}
	if (material) {
		disableColumns();
		ImGui::Separator();
		ImGui::Text("Shader pipeline: %s", model->getModel()->getMesh(0)->getShader()->getPipeline()->getName().c_str()); ImGui::NextColumn();
		ImGui::Separator();
		enableColumns();

		model->renderEditorGui(this);
	}

	// ================================
	//			TRANSFORM GUI
	// ================================
	TransformComponent* transform = entity->getComponent<TransformComponent>();
	if (model && transform) {
		disableColumns();
		ImGui::Separator();
		ImGui::Text("Transform");
		enableColumns(90.f);

		transform->renderEditorGui(this);
	}

	disableColumns();

	ImGui::End();

}

void ModelViewerGui::addProperty(const char* label, std::function<void()> prop) {
	ImGui::PushID(m_propID++);
	ImGui::AlignTextToFramePadding();
	ImGui::Text(label);
	ImGui::NextColumn();
	if (m_setNextPropWidth)
		ImGui::SetNextItemWidth(m_propWidth);
	prop();
	ImGui::NextColumn();
	ImGui::PopID();
}

void ModelViewerGui::setOption(const std::string& optionName, bool value) {
	if (optionName == "setWidth") {
		m_setNextPropWidth = value;
	}
}

void ModelViewerGui::newSection(const std::string& title) {
	disableColumns();
	ImGui::Separator();
	ImGui::Text(title.c_str());
	enableColumns();
}

void ModelViewerGui::setupDockspace(float menuBarHeight) {
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

float ModelViewerGui::setupMenuBar(){
	static bool showDemoWindow;
	ImVec2 menuBarSize;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Duffle")) {
			ImGui::MenuItem("Show demo window", NULL, &showDemoWindow);
			if (ImGui::MenuItem("Switch to GameState", NULL)) { m_funcSwitchState(); }
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

void ModelViewerGui::enableColumns(float labelWidth) {
	ImGui::PushID(m_propID++);
	ImGui::Columns(2, "alignedList", false);  // 3-ways, no border
	ImGui::SetColumnWidth(0, labelWidth);
	m_propWidth = ImGui::GetColumnWidth(1) - 10;
	ImGui::PopID();
}

void ModelViewerGui::disableColumns() {
	ImGui::Columns(1);
}
