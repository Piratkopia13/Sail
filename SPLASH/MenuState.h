#pragma once

#include "Sail.h"
#include <string>
using namespace std;

// Currently only a copy of State, but if more menus are added,
// similar behavior between menuStates should reside here.

struct message {
	unsigned int playerID;
	string content;
};

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
	Application* m_app = nullptr;
	Input* m_input = nullptr;

	// Formatting settings
	unsigned int m_outerPadding = 15;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight = 52;
	// Render ImGui Stuff
	void renderPlayerList();
	void renderStartButton();
	void renderSettings();
	void renderChat();

	// Purely for testing
	void addTestData();
	
	// Handling the Players & Chat
	message* m_messages = nullptr;
	string* m_players = nullptr;
	unsigned int m_playerCount = 0;
	unsigned int m_playerLimit = 12;
	unsigned int m_messageCount = 0;
	unsigned int m_messageLimit = 50;
	bool playerJoined(string name);
	

	
};