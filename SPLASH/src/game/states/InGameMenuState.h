#pragma once
#include "Sail.h"

class InGameMenuState : public State {
public:
	InGameMenuState(StateStack& stack);
	~InGameMenuState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	bool update(float dt, float alpha = 1.0f);

	bool fixedUpdate(float dt);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	//bool renderImgui(float dt);
	// Sends events to the state
	bool onEvent(Event& event) { return true; }
private:

};
