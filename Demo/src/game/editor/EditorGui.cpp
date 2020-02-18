#include "EditorGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

EditorGui::EditorGui() {
	m_modelName = "None - click to load";
}

void EditorGui::render(float dt, FUNC(void(CallbackType type, const std::string&)) callback) {
	newFrame();
	m_callback = callback;

	float menuBarHeight = setupMenuBar();

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		setupDockspace(menuBarHeight);
	}

	// Environment settings window
	if (ImGui::Begin("Environment")) {
		enableColumns(90.f);
		addProperty("Environment", [&]() {
			const char* environments[] = { "studio", "rail", "pier" };
			static int currentEnvironmentIndex = 0;
			ImGui::Combo("##hideLabel", &currentEnvironmentIndex, environments, IM_ARRAYSIZE(environments));
			m_callback(CallbackType::ENVIRONMENT_CHANGED, environments[currentEnvironmentIndex]);
		});
		disableColumns();
	}
	ImGui::End();
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
	static bool showDemoWindow = false;
	static bool showResourcesWindow = false;
	static bool showSettingsWindow = false;
	ImVec2 menuBarSize;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Duffle")) {
			ImGui::MenuItem("Resource tracker", NULL, &showResourcesWindow);
			ImGui::MenuItem("Settings", NULL, &showSettingsWindow);
			ImGui::MenuItem("Show demo window", NULL, &showDemoWindow);
			if (ImGui::MenuItem("Switch to GameState", NULL)) { m_callback(CallbackType::CHANGE_STATE, ""); }
			if (ImGui::MenuItem("Exit", NULL)) { PostQuitMessage(0); }
			ImGui::EndMenu();
		}

		menuBarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	if (showDemoWindow)
		ImGui::ShowDemoWindow();

	if (showResourcesWindow)
		m_resourceManagerGui.render();

	if (showSettingsWindow)
		m_settingsGui.render();

	return menuBarSize.y;
}

