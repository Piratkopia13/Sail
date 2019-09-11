#pragma once

#include "Sail.h"
#include <string>
#include <list>
using namespace std;

class TextInputEvent;

struct message {
	string sender;
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
	bool onEvent(Event& event);

private:
	std::unique_ptr<ImGuiHandler> m_imGuiHandler;
	Application* m_app = nullptr;
	Input* m_input = nullptr;

	// Render ImGui Stuff
	unsigned int m_outerPadding;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight;
	void renderPlayerList();
	void renderStartButton();
	void renderSettings();
	void renderChat();

	// Purely for testing
	void addTestData();
	void doTestStuff();
	
	// Handling the Players & Chat
	message* m_messages = nullptr;
	std::list<message> messages;
	std::list<string> players;
	string* m_players = nullptr;
	char* m_currentMessage = nullptr;
	unsigned int m_currentMessageIndex;
	unsigned int m_messageSizeLimit;
	unsigned int m_playerCount = 0;
	unsigned int m_playerLimit;
	unsigned int m_messageCount = 0;
	unsigned int m_messageLimit;
	string m_myName;
	bool firstFrame = true;

	// Events
	bool onTextInput(TextInputEvent& event);

	bool playerJoined(string name);
	bool playerLeft(string name);
	void sendMessage(string text);
	//bool playerLeft(string name); ? 

	
};