#include "pch.h"
#include "InGameMenuWindow.h"
#include "Sail/Application.h"

InGameMenuWindow::InGameMenuWindow(bool showWindow)
	: m_popGameState(false)
	, m_exitInGameMenu(false)
	, m_showOptions(false)
{
	Input::HideCursor(false);
}

InGameMenuWindow::~InGameMenuWindow() {}

void InGameMenuWindow::renderWindow() {
	// Rendering a pause window in the middle of the game window.
	ImGui::Begin("Pause Menu", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	
	ImVec2 size = ImVec2(140.f, 0.f);
	ImVec2 pos = ImVec2((Application::getInstance()->getWindow()->getWindowWidth() / 2.f) - (size.x / 2.f), (Application::getInstance()->getWindow()->getWindowHeight() / 2.f) - (size.y / 2.f));
	ImGui::SetWindowPos(pos);
	ImGui::SetWindowSize(size);

	if (ImGui::Button("Close menu")) {
		m_exitInGameMenu = true;
		Input::HideCursor(true);
	}
	if (ImGui::Button("Options")) {
		m_showOptions = true;
	}
	if (ImGui::Button("Exit to menu")) {
		m_popGameState = true;
	}
	ImGui::End();
}

bool InGameMenuWindow::popGameState() {
	return m_popGameState;
}

bool InGameMenuWindow::exitInGameMenu() {
	return m_exitInGameMenu;
}

bool InGameMenuWindow::showOptions() {
	return m_showOptions;
}
