#include "Game.h"
#include "states/GameState.h"
#include "states/MenuState.h"
#include "states/LobbyHostState.h"
#include "states/LobbyClientState.h"
#include "states/PBRTestState.h"
#include "states/InGameMenuState.h"
#include "states/EndGameState.h"
#include "states/PerformanceTestState.h"

Game::Game(HINSTANCE hInstance)
	: Application(1280, 720, "Sail | Game Engine Demo", hInstance)
	, m_stateStack()
	
{
	// Register states
	registerStates();

	// Set starting state
#ifdef _PERFORMANCE_TEST
	m_stateStack.pushState(States::PerformanceTest);
#else
	m_stateStack.pushState(States::MainMenu);
#endif

}

Game::~Game() {
}

int Game::run() {
	// Start the game loop and return when game exits
	return Application::startGameLoop();
}

void Game::registerStates() {
	// Register all of the different states
	m_stateStack.registerState<GameState>(States::Game);
	m_stateStack.registerState<LobbyHostState>(States::HostLobby);
	m_stateStack.registerState<LobbyClientState>(States::JoinLobby);
	m_stateStack.registerState<MenuState>(States::MainMenu);
	m_stateStack.registerState<InGameMenuState>(States::Pause);
	m_stateStack.registerState<EndGameState>(States::EndGame);
	m_stateStack.registerState<PBRTestState>(States::PBRTest);
	m_stateStack.registerState<PerformanceTestState>(States::PerformanceTest);
}

void Game::dispatchEvent(Event& event) {
	Application::dispatchEvent(event);
	m_stateStack.onEvent(event);
}

void Game::applyPendingStateChanges() {
	m_stateStack.prepareStateChange();
	this->m_stateStack.applyPendingChanges();
}

void Game::processInput(float dt) {
	m_stateStack.processInput(dt);
}

void Game::update(float dt, float alpha) {
	m_stateStack.update(dt, alpha);
}

void Game::fixedUpdate(float dt) {
	m_stateStack.fixedUpdate(dt);
}

void Game::render(float dt, float alpha) {
	m_stateStack.render(dt, alpha);
}