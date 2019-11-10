#pragma once

#include "Sail.h"
#include <string>
#include <list>

struct Player;
class NWrapper;
class TextInputEvent;
class AudioComponent;

struct NetworkChatEvent;
struct NetworkJoinedEvent;
struct NetworkDisconnectEvent;

struct Message {
	Netcode::PlayerID senderID;
	std::string content;
};
#define HOST_ID 0

class LobbyState : public State {
public:
	LobbyState(StateStack& stack);
	virtual ~LobbyState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	virtual bool update(float dt, float alpha = 1.0f);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state

protected:
	Application* m_app = nullptr;
	Input* m_input = nullptr;
	NWrapper* m_network = nullptr;
	char* m_currentmessage = nullptr;
	int* m_settingBotCount = nullptr;

	std::list<std::string> m_messages;

	// Front-End Functions
	bool inputToChatLog(const MSG& msg);
	void resetCurrentMessage();

	std::string fetchMessage();
	void addMessageToChat(const Message& message);
	virtual bool onEvent(const Event& event) override;

private:
	std::unique_ptr<ImGuiHandler> m_imGuiHandler;

	// LobbyAudio
	Entity* m_lobbyAudio = nullptr;

	// Back-end variables
	unsigned int m_currentmessageIndex;
	unsigned int m_messageSizeLimit;
	unsigned int m_messageCount;
	unsigned int m_messageLimit;
	bool m_firstFrame = true;	// Used solely for ImGui
	bool m_chatFocus = true;	// Used solely for ImGui
	unsigned int m_tempID = 0; // used as id counter until id's are gotten through network shit.

	// Render ImGui Stuff --------- WILL BE REPLACED BY OTHER GRAPHICS.
	bool m_settingsChanged;
	float m_timeSinceLastUpdate;

	unsigned int m_outerPadding;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight;

	virtual bool onMyTextInput(const TextInputEvent& event) = 0;
	bool onRecievedText(const NetworkChatEvent& event);
	bool onPlayerJoined(const NetworkJoinedEvent& event);
	bool onPlayerDisconnected(const NetworkDisconnectEvent& event);

	void renderPlayerList();
	void renderStartButton();
	void renderQuitButton();
	void renderSettings();
	void renderChat();
};