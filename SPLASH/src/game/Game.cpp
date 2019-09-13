#include "Game.h"
#include "states/GameState.h"
#include "Network/NetworkWrapper.h"

Game::Game(HINSTANCE hInstance)
	: Application(1280, 720, "Sail | Game Engine Demo", hInstance)
	, m_stateStack()
	
{
	// Register states
	registerStates();
	// Set starting state
	m_stateStack.pushState(States::Game);
	
	// Initialize the Network wrapper instance.
	NetworkWrapper::getInstance().Initialize();
}

Game::~Game() {
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

	if (NetworkWrapper::getInstance().isInitialized())
	{
		NetworkWrapper::getInstance().checkForPackages();
	}
	
	if (Input::GetInstance()->IsKeyPressed(SAIL_KEY_H)) {

		if (NetworkWrapper::getInstance().host())
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

		if (NetworkWrapper::getInstance().connectToIP("127.0.0.1:54000"))
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
		NetworkWrapper::getInstance().sendChatMsg("Kanel finns nu.");
	}

}

void Game::render(float dt) {
	m_stateStack.render(dt);
}