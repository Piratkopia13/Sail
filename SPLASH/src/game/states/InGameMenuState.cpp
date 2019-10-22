#include "InGameMenuState.h"
#include "imgui.h"
#include "Sail/api/Input.h"
#include "Sail/Application.h"
#include "Network/NWrapperSingleton.h"

InGameMenuState::InGameMenuState(StateStack& stack) : State(stack) {

}

InGameMenuState::~InGameMenuState(){}

bool InGameMenuState::processInput(float dt) {
	if (m_inGameMenuWindow.popGameState()) {
		// Reset the network
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();

		this->requestStackPop();
		this->requestStackPop();
		this->requestStackPush(States::MainMenu);
	}
	return true;
}

bool InGameMenuState::update(float dt, float alpha) {
	return false;
}

bool InGameMenuState::fixedUpdate(float dt) {
	return false;
}

bool InGameMenuState::render(float dt, float alpha) {
	return false;
}

bool InGameMenuState::renderImgui(float dt) {
	m_inGameMenuWindow.renderWindow();
	return false;
}