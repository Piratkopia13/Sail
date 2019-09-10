#pragma once

#include "MenuState.h"

class LobbyState : public MenuState {
public:
	LobbyState(StateStack& stack);
	virtual ~LobbyState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	bool update(float dt);
	// Renders the state
	bool render(float dt);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	bool onEvent(Event& event) { return true; }

private:


};
