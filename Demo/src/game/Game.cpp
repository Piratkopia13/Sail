#include "Game.h"
#include "states/GameState.h"
#include "states/ModelViewerState.h"

Game::Game(HINSTANCE hInstance)
	: Application(1600, 900, "Sail | Game Engine Demo", hInstance)
	, m_stateStack()
	
{
	// Register states
	registerStates();
	// Set starting state
	m_stateStack.pushState(States::Editor);

	// Set default settings
	getSettings().set(Settings::Graphics_SSAO, true);
}

Game::~Game() {	}

int Game::run() {
	// Start the game loop and return when game exits
	return startGameLoop();
}

void Game::registerStates() {
	// Register all of the different states
	m_stateStack.registerState<GameState>(States::Game);
	m_stateStack.registerState<ModelViewerState>(States::Editor);
}

void Game::processInput(float dt) {
	m_stateStack.processInput(dt);
}

void Game::update(float dt) {
	m_stateStack.update(dt);
}

void Game::render(float dt) {
	m_stateStack.render(dt);
}