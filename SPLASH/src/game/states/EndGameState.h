#pragma once

#include "Sail/states/State.h"

class EndGameState : public State {
public:
	explicit EndGameState(StateStack& stack);
	~EndGameState();

	// Process input for the state
	bool processInput(float dt) override;
	// Updates the state
	bool update(float dt, float alpha = 1.0f) override;
	// Renders the state
	bool render(float dt, float alpha = 1.0f) override;
	// Renders imgui
	bool renderImgui(float dt) override;
	// Sends events to the state
	bool onEvent(Event& event) override { return true; }

private:

};
