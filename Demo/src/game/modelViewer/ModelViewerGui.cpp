#include "ModelViewerGui.h"
#include "Sail.h"
#include <imgui/imgui.cpp>
#include <commdlg.h>
#include <atlstr.h>
#include <glm/gtc/type_ptr.hpp>

ModelViewerGui::ModelViewerGui() {
	m_modelName = "None - click to load";
}

void ModelViewerGui::render(float dt, FUNC(void()) funcSwitchState, FUNC(void(const std::string&)) callbackNewModel, PhongMaterial* material) {
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

	float propWidth = 0;
	int propID = 0;
	auto addProperty = [&](const char* label, std::function<void()> prop, bool setWidth = true) {
		ImGui::PushID(propID++);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(label);
		ImGui::NextColumn();
		if (setWidth)
			ImGui::SetNextItemWidth(propWidth);
		prop();
		ImGui::NextColumn();
		ImGui::PopID();
	};
	auto disableColumns = [] {
		ImGui::Columns(1);
	};
	auto enableColumns = [] {
		ImGui::Columns(2, "alignedList", false);  // 3-ways, no border
	};

	enableColumns();
	float width = ImGui::GetColumnWidth(1);
	ImGui::SetColumnWidth(0, 75);
	propWidth = ImGui::GetColumnWidth(1) - 10;

	limitStringLength(m_modelName);
	addProperty("Model", [&]() {
		if (ImGui::Button(m_modelName.c_str(), ImVec2(propWidth, 0))) {
			std::string newModel = openFilename(L"FBX models (*.fbx)\0*.fbx");
			if (!newModel.empty()) {
				m_modelName = newModel;
				m_callbackNewModel(m_modelName);
			}
		}
	});
		
	static const char* lbl = "##hidelabel";
	if (material) {
		disableColumns();
		ImGui::Separator();
		ImGui::Text("Shader pipeline: %s", material->getShader()->getPipeline()->getName().c_str()); ImGui::NextColumn();
		ImGui::Separator();
		enableColumns();

		addProperty("Ambient", [&] { ImGui::SliderFloat(lbl, &material->getPhongSettings().ka, 0.f, 10.f); });
		addProperty("Diffuse", [&] { ImGui::SliderFloat(lbl, &material->getPhongSettings().kd, 0.f, 10.f); });
		addProperty("Specular", [&] { ImGui::SliderFloat(lbl, &material->getPhongSettings().ks, 0.f, 10.f); });

		addProperty("Shininess", [&] { ImGui::SliderFloat(lbl, &material->getPhongSettings().shininess, 0.f, 10.f); });

		addProperty("Color", [&] { ImGui::ColorEdit4(lbl, glm::value_ptr(material->getPhongSettings().modelColor)); });

		disableColumns();
		ImGui::Separator();
		ImGui::Text("Textures");
		enableColumns();

		std::string diffuseTexName = (material->getTexture(0)) ? material->getTexture(0)->getName() : "None - click to load";
		limitStringLength(diffuseTexName);
		addProperty("Diffuse", [&]() {
			if (ImGui::Button(diffuseTexName.c_str(), ImVec2(propWidth - 15 - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = openFilename(L"TGA textures (*.tga)\0*.tga");
				if (!filename.empty()) {
					material->setDiffuseTexture(filename, true);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(lbl, ImVec2(15, 0))) {
				material->setDiffuseTexture("");
			}
		}, false);

		std::string normalTexName = (material->getTexture(1)) ? material->getTexture(1)->getName() : "None - click to load";
		limitStringLength(normalTexName);
		addProperty("Normal", [&]() {
			if (ImGui::Button(normalTexName.c_str(), ImVec2(propWidth - 15 - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = openFilename(L"TGA textures (*.tga)\0*.tga");
				if (!filename.empty()) {
					material->setNormalTexture(filename, true);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(lbl, ImVec2(15, 0))) {
				material->setNormalTexture("");
			}
		}, false);

		std::string specularTexName = (material->getTexture(2)) ? material->getTexture(2)->getName() : "None - click to load";
		limitStringLength(specularTexName);
		addProperty("Specular", [&]() {
			if (ImGui::Button(specularTexName.c_str(), ImVec2(propWidth - 15 - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = openFilename(L"TGA textures (*.tga)\0*.tga");
				if (!filename.empty()) {
					material->setSpecularTexture(filename, true);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(lbl, ImVec2(15, 0))) {
				material->setSpecularTexture("");
			}
		}, false);
	}

	disableColumns();

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

void ModelViewerGui::limitStringLength(std::string& str, int maxLength) {
	if (str.length() > maxLength) {
		str = "..." + str.substr(str.length() - maxLength + 3, maxLength - 3);
	}
}
