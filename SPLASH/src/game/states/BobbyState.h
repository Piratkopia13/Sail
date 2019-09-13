#pragma once

#include "Sail.h"
#include <string>
#include <list>
using namespace std;

class TextInputEvent;
class NetworkJoinedEvent;

struct Bmessage {
	string sender;
	string content;
};

struct Bplayer {
	unsigned int id;
	string name;

	bool friend operator==(const Bplayer& left, const Bplayer& right) {
		if (left.id == right.id &&
			left.name == right.name) {
			return true;
		}
		return false;
	}
};

class BobbyState : public State {
public:
	BobbyState(StateStack& stack);
	virtual ~BobbyState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	virtual bool update(float dt);
	// Renders the state
	bool render(float dt);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	virtual bool onEvent(Event& event) = 0;

protected:
	// Events
	virtual bool onTextInput(TextInputEvent& event) = 0;

	// Front-End Functions
	void inputToChatLog(MSG& msg);
	bool BplayerJoined(string name, unsigned int id);
	bool BplayerLeft(unsigned int id);

private:
	std::unique_ptr<ImGuiHandler> m_imGuiHandler;
	Application* m_app = nullptr;
	Input* m_input = nullptr;

	// Front-end functions - Use these!
	bool BplayerJoined(string name, unsigned int id);
	
	void sendBmessage(const string* text);
	void recieveBmessage(string text, unsigned int senderID);
	// Back-end functions
	void addBmessageToChat(const string* text, const Bplayer* sender);
	Bplayer* getBplayer(unsigned int id);
	// Back-end variables
	string m_myName;
	std::list<Bmessage> m_Bmessages;
	std::list<Bplayer> m_Bplayers;
	char* m_currentBmessage = nullptr;
	unsigned int m_currentBmessageIndex;
	unsigned int m_BmessageSizeLimit;
	unsigned int m_BplayerCount;
	unsigned int m_BplayerLimit;
	unsigned int m_BmessageCount;
	unsigned int m_BmessageLimit;
	bool m_firstFrame = true;	// Used solely for ImGui
	bool m_chatFocus = true;	// Used solely for ImGui
	unsigned int m_tempID = 0; // used as id counter until id's are gotten through network shit.

	// Purely for testing
	void addTestData();
	void doTestStuff();

	// Render ImGui Stuff --------- WILL BE REPLACED BY OTHER GRAPHICS.
	unsigned int m_outerPadding;
	unsigned int m_screenWidth;
	unsigned int m_screenHeight;
	unsigned int m_textHeight;
	void renderBplayerList();
	void renderStartButton();
	void renderSettings();		// Currently empty
	void renderChat();
};