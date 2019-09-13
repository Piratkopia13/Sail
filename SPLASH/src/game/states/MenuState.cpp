#include "MenuState.h"

#include "Sail.h"
#include "Network/NetworkWrapper.h"

MenuState::MenuState(StateStack& stack) 
	: State(stack)
{
	m_input = Input::GetInstance();
}

MenuState::~MenuState() {
}

bool MenuState::processInput(float dt) {

	// TEMPORARY TESTING FOR NETWORK
	if (NetworkWrapper::getInstance().isInitialized())
	{
		NetworkWrapper::getInstance().checkForPackages();
	}

	// -------- HOST -----------
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_H)) {

		if (NetworkWrapper::getInstance().host())
		{
			printf("Setting up host.");
			
			this->requestStackPop();
			this->requestStackPush(States::Lobby);
		}
		else
		{
			printf("Failed to set up Host.");
		}
	}
	// -------- JOIN -----------
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_J))
	{

		printf("Attempting connection... \n");

		if (NetworkWrapper::getInstance().connectToIP("127.0.0.1:54000"))
		{
			printf("Connecting to 192.168.1.55. \n");
			this->requestStackPop();
			this->requestStackPush(States::Lobby);
		}
		else
		{
			printf("Failed to connect. \n");
		}
	}
	// -------- MESSAGE -----------
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_M))
	{
		NetworkWrapper::getInstance().sendChatMsg("Kanel finns nu.");
	}

	return false;
}

bool MenuState::update(float dt) {
	return false;
}

bool MenuState::render(float dt) {
	return false;
}

bool MenuState::renderImgui(float dt) {
	return false;
}
