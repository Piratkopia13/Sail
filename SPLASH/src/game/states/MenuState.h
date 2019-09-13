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

typedef double long doong;

struct player {
	unsigned int id;
	string name;

	bool friend operator==(const player& left, const player& right) {
		if (left.id == right.id &&
			left.name == right.name) {
			return true;
		}
		return false;
	}
};

/*



*/

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

	// Front-end functions - Use these!
	bool playerJoined(string name, unsigned int id);
	bool playerLeft(unsigned int id);
	void sendMessage(const string* text);
	void recieveMessage(string text, unsigned int senderID);
	// Back-end functions
	void addMessageToChat(const string* text, const player* sender);
	player* getPlayer(unsigned int id);
	// Back-end variables
	string m_myName;
	std::list<message> m_messages;
	std::list<player> m_players;
	char* m_currentMessage = nullptr;
	unsigned int m_currentMessageIndex;
	unsigned int m_messageSizeLimit;
	unsigned int m_playerCount;
	unsigned int m_playerLimit;
	unsigned int m_messageCount;
	unsigned int m_messageLimit;
	bool m_firstFrame = true;	// Used solely for ImGui
	bool m_chatFocus = true;	// Used solely for ImGui
	unsigned int m_tempID = 0; // used as id counter until id's are gotten through network shit.

	// Events
	bool onTextInput(TextInputEvent& event);

	// Purely for testing
	void addTestData();
	void doTestStuff();
	
	// Render ImGui Stuff --------- WILL BE REPLACED BY OTHER GRAPHICS.
	unsigned int m_outerPadding;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight;
	void renderPlayerList();
	void renderStartButton();
	void renderSettings();		// Currently empty
	void renderChat();	
};