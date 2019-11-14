#include "InGameMenuState.h"
#include "imgui.h"
#include "Sail/api/Input.h"
#include "Sail/Application.h"
#include "Network/NWrapperSingleton.h"
#include "../events/GameOverEvent.h"
#include "Sail/events/EventDispatcher.h"
#include "Sail/utils/SailImGui/SailImGui.h"
bool InGameMenuState::sIsOpen = false;

InGameMenuState::InGameMenuState(StateStack& stack) 
	: State(stack) ,
	m_outerPadding(30)
{
	Input::HideCursor(false);
	m_app = Application::getInstance();
	m_imguiHandler = m_app->getImGuiHandler();
	sIsOpen = true;
	m_isSinglePlayer = NWrapperSingleton::getInstance().getPlayers().size() == 1;
	m_openedThisFrame = true;
	
	m_standaloneButtonflags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBackground;
	m_backgroundOnlyflags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

}

InGameMenuState::~InGameMenuState(){
	sIsOpen = false;
}

bool InGameMenuState::processInput(float dt) {
#ifndef DEVELOPMENT
	// Show mouse
#endif
	if (m_inGameMenuWindow.popGameState()) {
		
	} else if (m_inGameMenuWindow.exitInGameMenu()) {
		this->requestStackPop();
	}

	return false;
}

bool InGameMenuState::update(float dt, float alpha) {
	return !m_isSinglePlayer;
}

bool InGameMenuState::fixedUpdate(float dt) {
	return !m_isSinglePlayer;
}

bool InGameMenuState::render(float dt, float alpha) {
	return true;
}

bool InGameMenuState::renderImgui(float dt) {


	renderMenu();
	if (m_inGameMenuWindow.showOptions()) {
		renderOptions();
	}
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({
		(float)m_app->getWindow()->getWindowWidth(),
		(float)m_app->getWindow()->getWindowHeight()
		});
	if (ImGui::Begin("BACKGROUND", nullptr, m_backgroundOnlyflags)) {

	}
	ImGui::End();
	return false;
}

void InGameMenuState::renderMenu() {
	ImVec2 size = ImVec2(400.f, 0.f);
	//ImVec2 pos = ImVec2((Application::getInstance()->getWindow()->getWindowWidth() / 2.f) - (size.x / 2.f), (Application::getInstance()->getWindow()->getWindowHeight() / 2.f) - (size.y / 2.f));
	ImVec2 pos = ImVec2(
		m_outerPadding,
		m_outerPadding
	);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushFont(m_imguiHandler->getFont("Beb60"));

	ImGui::Begin("##Pause Menu", nullptr, m_standaloneButtonflags);

	
	if (SailImGui::TextButton("Close menu") || (!m_openedThisFrame && ImGui::IsKeyPressed(KeyBinds::SHOW_IN_GAME_MENU))) {
		Input::HideCursor(true);
		this->requestStackPop();
	}
	if (SailImGui::TextButton("Options")) {
		//m_showOptions = true;
	}
	if (SailImGui::TextButton("Exit to main menu")) {
		// Reset the network
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();
		// Dispatch game over event
		EventDispatcher::Instance().emit(GameOverEvent());

		this->requestStackClear();
		this->requestStackPush(States::MainMenu);
	}
	ImGui::End();
	ImGui::PopFont();

}

void InGameMenuState::renderOptions() {

	ImVec2 size(500, m_app->getWindow()->getWindowHeight() - m_outerPadding * 2);
	ImGui::SetNextWindowPos(ImVec2(
		m_app->getWindow()->getWindowWidth() - m_outerPadding - size.x,
		m_outerPadding
	));
	ImGui::SetNextWindowSize(size);

	if (ImGui::Begin("##OptionsWindow", nullptr, m_backgroundOnlyflags)) {
		m_optionsWindow.renderWindow();
	}
	ImGui::End();



}

bool InGameMenuState::IsOpen() {
	return sIsOpen;
}
