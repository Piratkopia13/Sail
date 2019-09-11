#include "Game.h"
#include "states/GameState.h"
#include "../NetworkWrapper.h"

Game::Game(HINSTANCE hInstance)
	: Application(1280, 720, "Sail | Game Engine Demo", hInstance)
	, m_stateStack()
	
{
	// Register states
	registerStates();
	// Set starting state
	m_stateStack.pushState(States::Game);
	
	m_networkWrapper = new NetworkWrapper();
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
		m_networkWrapper->CheckForPackages();
	}
	
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_H)) {
		
		if (m_networkWrapper->Host())
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
		if (m_networkWrapper->ConnectToIP("192.168.1.55"))
		{
			printf("Connecting to 192.168.1.55.");
		}
		else
		{
			printf("Failed to connect.");
		}
	}

	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_M))
	{
		m_networkWrapper->SendChat("mHoppla");
	}



}

void Game::render(float dt) {
	m_stateStack.render(dt);
}