#include "MenuState.h"


//#include "../imgui-sfml-master/imgui-SFML.h"
#include "..//libraries/imgui/imgui.h"
#include "..//Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
#include "../Sail/src/API/Windows/Win32Input.h"
#include <string>
using namespace std;




MenuState::MenuState(StateStack& stack)
: State(stack)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_playerLimit = 12;
	m_messageLimit = 5;
	
	m_messages = new message[m_messageLimit];
	m_players = new string[m_playerLimit];


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
	if (m_input->IsKeyPressed(SAIL_KEY_DOWN)) {
		// 48-90 0-Z
	}

	// Did user want to change some setting?
	// ---

	// Did user want to start the game?
	// ---

	return false;
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

	// ------- CHAT LOG ENTER MSG ------- 
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

	for (size_t i = 0; i < m_playerCount; i++) {
		ImGui::Text(
			std::string("- ").append(m_players[i].c_str()).c_str()
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
	ImGui::Begin("Goto GameState", NULL, flags);
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

	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (m_outerPadding + m_textHeight)
	));
	ImGui::Begin(
		"Write Here",
		NULL,
		chatFlags
	);
	char* input = new char[255]{ 0 };
	int charCount = 255;
	ImGui::Text("Enter Message:");

	if (ImGui::InputText("", input, 255, ImGuiInputTextFlags_EnterReturnsTrue)) {

		// Save messages here, use a messageLogger or smth which
		// handles all responsibilities for logging and maintaining
		// a fixed size, deletion of extra, etc.
		// ---
		std::string temp = std::string(input);

		message msg = {
			0, // My id
			temp
		};
		m_messageCount = m_messageCount % m_messageLimit;
		m_messages[m_messageCount++] = msg;
	}
	delete[]input;

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


	ImGui::Begin("Chat Log", NULL, chatFlags);
	ImGui::SameLine();
	ImGui::BeginChild("Messages");

	for (size_t i = 0; i < m_messageCount; i++) {
		std::string msg;
		msg.append("HH:MM:SS | ");
		msg.append(m_players[m_messages[i].playerID]);
		msg.append(" | ");
		msg.append(m_messages[i].content.c_str()).c_str();
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
	
	message msg;
	msg.playerID = 0;
	msg.content = "msg1";
	m_messages[0] = msg;

	msg.playerID = 1;
	msg.content = "Message Two";
	m_messages[1] = msg;

	msg.playerID = 2;
	msg.content = "msg 3 mr.boss";
	m_messages[2] = msg;
}

bool MenuState::playerJoined(string name) {
	if (m_playerCount < m_playerLimit) {
		m_players[m_playerCount++] = name;
		return true;
	}
	return false;
}
