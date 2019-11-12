#pragma once

#include "Sail.h"
#include <string>
#include <list>
#include "Sail/utils/SailImGui/OptionsWindow.h"

struct Player;
class NWrapper;
class TextInputEvent;
class NetworkJoinedEvent;
class AudioComponent;

struct Message {
	std::string sender;
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
	virtual bool onEvent(const Event& event) override = 0;

protected:
	Application* m_app = nullptr;
	Input* m_input = nullptr;
	NWrapper* m_network = nullptr;
	SettingStorage* m_settings;
	char* m_currentmessage = nullptr;
	int* m_settingBotCount = nullptr;

	std::list<Message> m_messages;

	// Front-End Functions
	bool inputToChatLog(const MSG& msg);
	void addTextToChat(const Message& text);
	void resetCurrentMessage();

	std::string fetchMessage();
	void addMessageToChat(const Message& message);

private:
	ImGuiHandler* m_imGuiHandler;
	OptionsWindow m_optionsWindow;
	bool m_ready;
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

	ImGuiWindowFlags m_standaloneButtonflags;



	unsigned int m_outerPadding;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight;

	bool m_renderGameSettings;
	bool m_renderApplicationSettings;

	void renderPlayerList();
	void renderGameSettings();		// Currently empty
	void renderChat();
	void renderMenu();
};