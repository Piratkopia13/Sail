#include "pch.h"
#include "KillFeedWindow.h"

#include "Sail/Application.h"

KillFeedWindow::KillFeedWindow(bool showWindow) {}

KillFeedWindow::~KillFeedWindow() {}

void KillFeedWindow::setDeaths(const std::vector<std::string>& kills) {
	m_kills = kills;
}

void KillFeedWindow::renderWindow() {
	if (m_kills.size() > 0) {
		ImVec2 minSize = ImVec2(0.f, 0.f);
		ImVec2 maxSize = ImVec2(240.f, 200.f);
		ImGui::SetNextWindowSizeConstraints(minSize, maxSize);

		ImVec2 textMinSize = ImVec2(0, 0);
		ImVec2 textMaxSize = ImVec2(240.f, 20.f);
		ImGui::Begin("Kill Feed", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.05f));
		for (int i = m_kills.size() - 1; i > -1; i--) {
			ImGui::SetNextWindowSizeConstraints(textMinSize, textMaxSize);
			ImGui::BeginChild("", ImVec2(0.f, 0.f), false, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text(m_kills[i].c_str());
			ImGui::EndChild();
		}
		ImGui::PopStyleColor(1);

		ImVec2 size = ImGui::GetWindowSize();
		ImVec2 pos = ImVec2(Application::getInstance()->getWindow()->getWindowWidth() - size.x, 0.f);
		ImGui::SetWindowPos(pos);

		ImGui::End();
	}
}
