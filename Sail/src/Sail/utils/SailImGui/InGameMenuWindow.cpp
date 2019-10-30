#include "pch.h"
#include "InGameMenuWindow.h"
#include "Sail/Application.h"
#include "Sail/KeyBinds.h"

InGameMenuWindow::InGameMenuWindow(bool showWindow)
	: m_popGameState(false)
	, m_exitInGameMenu(false)
	, m_showOptions(false)
{
	m_openedThisFrame = true;
	Input::HideCursor(false);
}

InGameMenuWindow::~InGameMenuWindow() {}

void InGameMenuWindow::renderWindow() {
	// Rendering a pause window in the middle of the game window.
	ImGui::Begin("Pause Menu", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	
	ImVec2 size = ImVec2(180.f, 0.f);
	ImVec2 pos = ImVec2((Application::getInstance()->getWindow()->getWindowWidth() / 2.f) - (size.x / 2.f), (Application::getInstance()->getWindow()->getWindowHeight() / 2.f) - (size.y / 2.f));
	ImGui::SetWindowPos(pos);
	ImGui::SetWindowSize(size);

	if (ImGui::Button("Close menu") || (!m_openedThisFrame && ImGui::IsKeyPressed(KeyBinds::SHOW_IN_GAME_MENU))) {
		m_exitInGameMenu = true;
		Input::HideCursor(true);
	}
	if (ImGui::Button("Options")) {
		m_showOptions = true;
	}
	if (ImGui::Button("Exit to main menu")) {	
		m_popGameState = true;
	}
	ImGui::End();
	m_openedThisFrame = false;
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
