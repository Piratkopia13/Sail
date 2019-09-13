#include "Game.h"
#include "states/GameState.h"
#include "states/MenuState.h"
#include "../NetworkWrapper.h"

Game::Game(HINSTANCE hInstance)
	: Application(1280, 720, "Sail | Game Engine Demo", hInstance)
	, m_stateStack()
	
{
	// Register states
	registerStates();

	// Set starting state
//	m_stateStack.pushState(States::Game);
	m_stateStack.pushState(States::MainMenu);
}

Game::~Game() {
	delete m_networkWrapper;
}

int Game::run() {
	
	// Start the game loop and return when game exits
	return startGameLoop();
}

void Game::registerStates() {
	// Register all of the different states
	m_stateStack.registerState<GameState>(States::Game);
	m_stateStack.registerState<MenuState>(States::MainMenu);
}

void Game::dispatchEvent(Event& event) {
	Application::dispatchEvent(event);
	m_stateStack.onEvent(event);
}

void Game::processInput(float dt) {
	m_stateStack.processInput(dt);
}

void Game::update(float dt) {
	m_stateStack.update(dt);

	// TEMPORARY TESTING FOR NETWORK

	if (m_networkWrapper->isInitialized())
	{
		m_networkWrapper->checkForPackages();
	}
	
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_H)) {

		if (m_networkWrapper->host())
		{
			printf("Setting up host.");
		}
		else
		{
			printf("Failed to set up Host.");
		}
	}

	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_J))
	{
		printf("Attempting connection... \n");
		/*if (m_networkWrapper->connectToIP("192.168.1.55:54540"))
		{
			printf("Connecting to 192.168.1.55. \n");
		}*/
		if (m_networkWrapper->connectToIP("127.0.0.1:54000"))
		{
			printf("Connecting to 192.168.1.55. \n");
		}
		else
		{
			printf("Failed to connect. \n");
		}
	}

	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_M))
	{
		m_networkWrapper->sendChatMsg("Kanel finns nu.");
	}



}

void Game::render(float dt) {
	m_stateStack.render(dt);
}