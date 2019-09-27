#pragma once

#include "Sail.h"
#include <string>
#include <list>

class NWrapper;
class TextInputEvent;
class NetworkJoinedEvent;

struct Message {
	std::string sender;
	std::string content;
};

struct Player {
	unsigned char id;
	std::string name;

	bool friend operator==(const Player& left, const Player& right) {
		if (left.id == right.id &&
			left.name == right.name) {
			return true;
		}
		return false;
	}
};

#define HOST_ID 1337

class LobbyState : public State {
public:
	LobbyState(StateStack& stack);
	virtual ~LobbyState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	virtual bool update(float dt);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	virtual bool onEvent(Event& event) = 0;

protected:
	Application* m_app = nullptr;
	Input* m_input = nullptr;
	NWrapper* m_network = nullptr;
	char* m_currentmessage = nullptr;
	Player m_me;
	std::list<Message> m_messages;
	std::list<Player> m_players;
	Player* getPlayer(unsigned char& id);

	// Front-End Functions
	bool inputToChatLog(MSG& msg);
	void resetPlayerList();
	bool playerJoined(Player& player);
	bool playerLeft(unsigned char& id);
	void addTextToChat(Message* text);
	void resetCurrentMessage();

	std::string fetchMessage();
	void addMessageToChat(Message& message);

private:
	std::unique_ptr<ImGuiHandler> m_imGuiHandler;

	// Back-end variables
	unsigned int m_currentmessageIndex;
	unsigned int m_messageSizeLimit;
	unsigned int m_playerCount;
	unsigned int m_playerLimit;
	unsigned int m_messageCount;
	unsigned int m_messageLimit;
	bool m_firstFrame = true;	// Used solely for ImGui
	bool m_chatFocus = true;	// Used solely for ImGui
	unsigned int m_tempID = 0; // used as id counter until id's are gotten through network shit.

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