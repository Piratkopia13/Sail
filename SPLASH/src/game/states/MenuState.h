#pragma once

#include "Sail.h"
#include "Network/NWrapperSingleton.h"
#include "Network/NWrapperHost.h"


class MenuState : public State, public NetworkEvent {

public:
	typedef std::unique_ptr<State> Ptr;

public:
	MenuState(StateStack& stack);
	virtual ~MenuState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	bool update(float dt);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	bool onEvent(Event& event) { return true; }

private:
	Input* m_input = nullptr;
	// NetworkWrapper | NWrapperSingleton | NWrapperHost
	NWrapperSingleton* m_network = nullptr;
	Application* m_app = nullptr;
	// For ImGui Input
	char* inputIP = nullptr;
	char* inputName = nullptr;
};