#include "MenuState.h"


//#include "../imgui-sfml-master/imgui-SFML.h"
#include "..//libraries/imgui/imgui.h"
#include "..//Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
#include "..//SPLASH/src/game/events/TextInputEvent.h"

#include <string>
using namespace std;




MenuState::MenuState(StateStack& stack)
: State(stack)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_textHeight = 52;
	m_outerPadding = 15;
	m_myName = "Daniel";

	m_messageLimit = 14;		// Should be based on size of chatbox
	m_messages = new message[m_messageLimit];

	m_playerLimit = 12;
	m_players = new string[m_playerLimit];

	m_messageSizeLimit = 50;
	m_currentMessageIndex = 0;
	m_currentMessage = new char[m_messageSizeLimit] { 0 };

	// -------- test -------- 
	addTestData();
}

MenuState::~MenuState()
{
	// Handle destruction of ImGui
	// ---
	delete[] this->m_messages;
	delete[] this->m_players;
}

bool MenuState::processInput(float dt){
	// Did user want to enter anything in the chatbox?
	// Did user want to send the message?
	// ---

	/// ChatBox input from here, so it always recieves input (maybe switch on/off with enter)
	this->doTestStuff();

	// Did user want to change some setting?
	// ---
	// Did user want to start the game?
	// ---

	return false;
}

bool MenuState::onEvent(Event& event) {
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&MenuState::onTextInput));

	return false;
}

bool MenuState::onTextInput(TextInputEvent& event)
{
	TextInputEvent* e = (TextInputEvent*) & event;
	MSG msg = e->getMSG();

	if (m_currentMessageIndex < m_messageSizeLimit && msg.wParam != SAIL_KEY_RETURN) {
		// Add whichever button that was inputted to the current message
		m_currentMessage[m_currentMessageIndex++] = (char)msg.wParam;
	}
	
	return true;
}


bool MenuState::update(float dt){
	// Update screen dimensions & ImGui related
	// (Sure, events, but the only thing consuming resources is the menustate)
	this->m_screenWidth = m_app->getWindow()->getWindowWidth();
	this->m_screenHeight = m_app->getWindow()->getWindowHeight();


	// Did we send something?
	// ---
	

	// Did we recieve something?
	// Append the message in the proper order in the chatbox.
	// ---

	// Is anything going on in the background?
	// ---

	// Did we send something?
	// ---

	return false;
}

bool MenuState::render(float dt) {
	/// Maybe have m_ptr instead of fetching instance every frame.
	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	


	return false;
}

bool MenuState::renderImgui(float dt) {
	// ------- PLAYER LIST ------- 
	renderPlayerList();

	// ------- CHAT LOG ------- 
	renderChat();

	// -------- SETTINGS ----------
	renderSettings();

	// ------- START BUTTON ------- 
	renderStartButton();

	return false;
}

void MenuState::renderPlayerList() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_outerPadding
	));
	ImGui::Begin("Players in lobby:", NULL, flags);

	/*for (size_t i = 0; i < players.size(); i++) {
		ImGui::Text(
			std::string("- ").append(m_players[i].c_str()).c_str()
		);
	}*/
	for (auto currentPlayer : players) {
		ImGui::Text(
			std::string("- ").append(currentPlayer.c_str()).c_str()
		);
	}
	ImGui::End();
}

void MenuState::renderStartButton() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowPos(ImVec2(
		m_screenWidth - (m_outerPadding + m_screenWidth / 10.0f),
		m_screenHeight - (m_outerPadding + m_screenHeight / 10.0f)
	));
	ImGui::Begin("Press 0 once", NULL, flags);

	// SetKeyBoardFocusHere on the chatbox prevents the button from working,
	// so if we click with the mouse, temporarily set focus to the button.

	if (ImGui::Button("S.P.L.A.S.H")) {
		// Queue a removal of menustate, then a push of gamestate
		this->requestStackPop();
		this->requestStackPush(States::Game);
	}
	ImGui::End();
}

void MenuState::renderSettings() {

}

void MenuState::renderChat() {
	ImGuiWindowFlags chatFlags = ImGuiWindowFlags_NoCollapse;
	chatFlags |= ImGuiWindowFlags_NoResize;
	chatFlags |= ImGuiWindowFlags_NoMove;
	chatFlags |= ImGuiWindowFlags_NoNav;
	chatFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	chatFlags |= ImGuiWindowFlags_NoTitleBar;
	chatFlags |= ImGuiWindowFlags_AlwaysAutoResize;

	// ------- MESSAGE BOX ------- 
	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (m_outerPadding + m_textHeight)
	));
	ImGui::Begin(
		"Write Here",
		NULL,
		chatFlags
	);

	if (firstFrame) {
		ImGui::SetKeyboardFocusHere(-1);
		firstFrame = false;
	}
	ImGui::Text("Enter Message:");
	if (ImGui::InputText("", m_currentMessage, m_messageSizeLimit, ImGuiInputTextFlags_EnterReturnsTrue)) {
		this->sendMessage(m_currentMessage);
		ImGui::SetKeyboardFocusHere(-1);
	}
	ImGui::End();

	// ------- CHAT LOG ------- 
	ImGui::SetNextWindowSize(ImVec2(
		400,
		300
	));
	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (300 + m_outerPadding)
	));

	// Render message history
	ImGui::Begin("Chat Log", NULL, chatFlags);
	ImGui::SameLine();
	ImGui::BeginChild("Messages");
	for (auto currentMessage : messages) {
		//std::string msg;
		//msg.append("HH:MM:SS | ");
		//msg.append(m_players[currentMessage.playerID]);
		//msg.append(" | ");
		//msg.append(currentMessage.content.c_str()).c_str();
		//ImGui::Text(
		//	msg.c_str()
		//);

		std::string msg;
		msg.append("HH:MM:SS | ");
		msg.append(currentMessage.sender);
		msg.append(" | ");
		msg.append(currentMessage.content.c_str()).c_str();
		ImGui::Text(
			msg.c_str()
		);
	}
	ImGui::EndChild();
	ImGui::End();
}

void MenuState::addTestData()
{
	// Set up players
	playerJoined("Daniel");
	playerJoined("Ollie");
	playerJoined("David");
	//playerJoined("Press 0 to switch between enter msg and going to gamestate with mouse (MouseClick by default)");
	//playerJoined("The cause of this is an IMGUI-bug where keyboard focus prevents buttons from working");
	//playerJoined("This tape is since we'll switch from imgui later anyway, so why patch shit that's only an imgui bug");
	//playerJoined("If u want to look at it: https://github.com/ocornut/imgui/issues/455#event-1428367449");
	
	message msg;
	msg.sender = "Daniel";
	msg.content = "msg1";
	m_messages[0] = msg;
	messages.push_back(msg);

	msg.sender = "Ollie";
	msg.content = "Message Two";
	m_messages[1] = msg;
	messages.push_back(msg);

	msg.sender = "David";
	msg.content = "msg 3 mr.boss";
	m_messages[2] = msg;
	messages.push_back(msg);
}

void MenuState::doTestStuff()
{
	if (m_input->WasKeyJustPressed(SAIL_KEY_J)) {
		playerJoined("TestPlayer");
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_U)) {
		playerLeft("Ollie");
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_I)) {
		playerJoined("David");
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_O)) {
		playerLeft ("TestPlayer");
	}
}


bool MenuState::playerJoined(string name) {
	/*if (m_playerCount < m_playerLimit) {
		m_players[m_playerCount++] = name;
		return true;
	}*/

	if (m_playerCount < m_playerLimit) {
		players.push_back(name);
		return true;
	}
	return false;
}

bool MenuState::playerLeft(string name) {
	players.remove(name);
	return true;
}

void MenuState::sendMessage(string text) {
	if (m_currentMessageIndex != 0) { // If the message isn't empty
		// Add message to chatlog
		messages.push_back(message{
			m_myName,
			text
		});

		// New messages replace old
		if (messages.size() > m_messageLimit) {
			messages.pop_front();
		}

		// Reset currentMessage
		m_currentMessageIndex = 0;
		*m_currentMessage = { 0 };
	}
}
