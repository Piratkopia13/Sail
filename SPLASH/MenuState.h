#pragma once

#include "Sail.h"

// Currently only a copy of State, but if more menus are added,
// similar behavior between menuStates should reside here.

class MenuState : public State {
public:
	MenuState(StateStack& stack);
	virtual ~MenuState();

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
	std::unique_ptr<ImGuiHandler> m_imGuiHandler;
	
};