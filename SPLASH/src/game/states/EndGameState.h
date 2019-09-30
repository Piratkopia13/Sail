#pragma once

#include "Sail/states/State.h"

class EndGameState : public State {
public:
	EndGameState(StateStack& stack);
	~EndGameState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	bool update(float dt, float alpha = 1.0f);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	bool onEvent(Event& event) { return true; }

private:

};
