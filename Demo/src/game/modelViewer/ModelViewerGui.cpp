#include "ModelViewerGui.h"
#include "Sail/Application.h"
#include <imgui/imgui.cpp>
#include <commdlg.h>
#include <atlstr.h>

ModelViewerGui::ModelViewerGui() {
	m_modelName = "None";
}

void ModelViewerGui::render(float dt, FUNC(void()) funcSwitchState, FUNC(void(const std::string&)) callbackNewModel) {
	m_funcSwitchState = funcSwitchState;
	m_callbackNewModel = callbackNewModel;

	float menuBarHeight = setupMenuBar();

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		setupDockspace(menuBarHeight);
	}

	// Model customization window
	ImGui::Begin("Model customizer");
	ImGui::Text("Current model: %s", m_modelName.c_str());
	if (ImGui::Button("Load new model")) {
		std::string newModel = openFilename();
		if (!newModel.empty()) {
			m_modelName = newModel;
			m_callbackNewModel(m_modelName);
		}
	}
	ImGui::End();

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
	ImVec2 menuBarSize;
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Potato")) {

			if (ImGui::MenuItem("Switch to GameState", NULL)) { m_funcSwitchState(); }
			if (ImGui::MenuItem("Exit", NULL)) { PostQuitMessage(0); }
			ImGui::EndMenu();
		}

		menuBarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	return menuBarSize.y;
}

std::string ModelViewerGui::openFilename(LPCWSTR filter, HWND owner) {
	OPENFILENAME ofn;
	WCHAR fileName[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"";
	std::string fileNameStr;
	if (GetOpenFileName(&ofn))
		fileNameStr = CW2A(fileName);
	return fileNameStr;
}