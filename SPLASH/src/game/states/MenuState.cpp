#include "MenuState.h"

#include "Sail.h"
#include "../libraries/imgui/imgui.h"

#include "Network/NWrapperSingleton.h"
#include "Network/NWrapper.h"

MenuState::MenuState(StateStack& stack) 
	: State(stack)
{
	m_input = Input::GetInstance();
	m_network = &NWrapperSingleton::getInstance();
	m_app = Application::getInstance();

	this->inputIP = new char[100]{ "127.0.0.1:54000" };
	this->inputName = new char[100]{ "Gottem420" };
}

MenuState::~MenuState() {
	delete this->inputIP;
	delete this->inputName;
	//delete m_network;
}

bool MenuState::processInput(float dt) {
	return false;
}

bool MenuState::update(float dt) {
	return false;
}

bool MenuState::render(float dt) {
	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	m_scene.draw();
	return false;
}

bool MenuState::renderImgui(float dt) {
	
	// Host
	ImGui::Begin("Host Game");
	if (ImGui::Button("S.P.L.A.S.H over here")) {
		if (m_network->host()) {
			printf("Setting up host.\n");
			this->requestStackPop();
			this->requestStackPush(States::HostLobby);
		}
	}
	ImGui::End();

	ImGui::Begin("Name:");
	ImGui::InputText("", inputName, 100);
	m_app->getStateStorage().setMenuToLobbyData(MenuToLobbyData{ inputName });
	ImGui::End();

	// 
	ImGui::Begin("Join Game");
	ImGui::InputText("IP:", inputIP, 100);
	if (ImGui::Button("S.P.L.A.S.H over there")) {
		if (m_network->connectToIP(inputIP)) {
			printf("Connecting to 192.168.1.55. \n");

			// Wait until welcome-package is recieved,
			// Save the package info,
			// Pop and push into JoinLobbyState.
			
			this->requestStackPop();
			this->requestStackPush(States::JoinLobby);
		}
	}
	ImGui::End();



	return false;
}
