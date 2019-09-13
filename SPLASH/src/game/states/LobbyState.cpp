//#include "LobbyState.h"
//
//
////#include "../imgui-sfml-master/imgui-SFML.h"
//#include "../libraries/imgui/imgui.h"
//#include "../Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
//#include "../SPLASH/src/game/events/TextInputEvent.h"
//#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
//#include "Network/NetworkWrapper.h"
//
//#include <string>
//using namespace std;
//
//// Nothing
//
//
//LobbyState::LobbyState(StateStack& stack)
//: State(stack)
//{
//	// ImGui is already initiated and set up, thanks alex!
//	m_app = Application::getInstance();
//	m_input = Input::GetInstance();
//	m_textHeight = 52;
//	m_outerPadding = 15;
//
//	m_messageLimit = 14;		// Should be based on size of chatbox instead of fixed
//	m_playerLimit = 12;
//	m_playerCount = 0;
//	m_messageCount = 0;
//
//	// Add local player as the first.
//	m_myName = "Daniel";
//	playerJoined(m_myName, m_tempID++);
//
//	m_messageSizeLimit = 50;
//	m_currentMessageIndex = 0;
//	m_currentMessage = new char[m_messageSizeLimit] { 0 };
//
//	// -------- test -------- 
//	//addTestData();
//}
//
//LobbyState::~LobbyState() {
//	delete []m_currentMessage;
//}
//
//bool LobbyState::processInput(float dt){
//	// Did user want to enter anything in the chatbox?
//	// Did user want to send the message?
//	// ---
//
//	if (m_input->IsMouseButtonPressed(0)) {
//		m_chatFocus = false;
//	}
//
//	// Purely a function for testing
////	this->doTestStuff();
//
//	// Did user want to change some setting?
//	// ---
//
//	// Did user want to start the game?
//	// ---
//
//	return false;
//}
//
//bool LobbyState::onEvent(Event& event) {
//	Logger::Log("Received event: " + std::to_string(event.getType()));
//
//	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&LobbyState::onTextInput));
//	EventHandler::dispatch<NetworkJoinedEvent>(event, SAIL_BIND_EVENT(&LobbyState::onPlayerJoined));
//
//
//	return false;
//}
//
//
//bool LobbyState::onPlayerJoined(NetworkJoinedEvent& event)
//{
//	return false;
//}
//
//bool LobbyState::onTextInput(TextInputEvent& event)
//{
//	TextInputEvent* e = (TextInputEvent*) & event;
//	MSG msg = e->getMSG();
//
//	if (m_currentMessageIndex < m_messageSizeLimit && msg.wParam != SAIL_KEY_RETURN) {
//		// Add whichever button that was inputted to the current message
//		// --- OBS : doesn't account for capslock, etc.
//		m_currentMessage[m_currentMessageIndex++] = (char)msg.wParam;
//	}
//	if (msg.wParam == SAIL_KEY_RETURN && m_chatFocus == false) {
//		sendMessage(&string(m_currentMessage));
//	}
//
//	return true;
//}
//
//
//bool LobbyState::update(float dt){
//	// Update screen dimensions & ImGui related
//	// (Sure, events, but the only thing consuming resources is the LobbyState)
//	this->m_screenWidth = m_app->getWindow()->getWindowWidth();
//	this->m_screenHeight = m_app->getWindow()->getWindowHeight();
//
//	// Did we send something?
//	// ---
//	if (NetworkWrapper::getInstance().isInitialized())
//	{
//		NetworkWrapper::getInstance().checkForPackages();
//	}
//
//	// Did we recieve something?
//	// ---
//
//	// Is anything going on in the background?
//	// ---
//
//	return false;
//}
//
//bool LobbyState::render(float dt) {
//	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
//
//	return false;
//}
//
//bool LobbyState::renderImgui(float dt) {
//	// ------- PLAYER LIST ------- 
//	renderPlayerList();
//
//	// ------- CHAT LOG ------- 
//	renderChat();
//
//	// -------- SETTINGS ----------
//	renderSettings();
//
//	// ------- START BUTTON ------- 
//	renderStartButton();
//
//	return false;
//}
//
//bool LobbyState::playerJoined(string name, unsigned int id) {
//	if (m_playerCount < m_playerLimit) {
//		player newPlayer{
//			id,
//			name
//		};
//		m_players.push_back(newPlayer);
//		m_playerCount++;
//		return true;
//	}
//	return false;
//}
//
//bool LobbyState::playerLeft(unsigned int id) {
//	// Linear search to get target 'player' struct, then erase that from the list
//	player* toBeRemoved = nullptr;
//	int pos = 0;
//	for (auto playerIt : m_players) {
//		if (playerIt.id == id) {
//			toBeRemoved = &playerIt;
//			m_players.remove(*toBeRemoved);
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void LobbyState::sendMessage(const string* text) {
//	if (m_currentMessageIndex != 0) { // If the message isn't empty
//		this->addMessageToChat(text, &m_players.front());
//
//		// Reset currentMessage
//		m_currentMessageIndex = 0;
//		for (size_t i = 0; i < m_messageSizeLimit; i++) {
//			m_currentMessage[i] = '\0';
//		}
//	}
//}
//
//void LobbyState::recieveMessage(string text, unsigned int senderID) {
//
//
//	addMessageToChat(&text, getPlayer(senderID));
//
//}
//
//void LobbyState::addMessageToChat(const string* text, const player* sender) {
//	// Add message to chatlog
//	m_messages.push_back(message{
//		sender->name,
//		*text
//	});
//
//	// New messages replace old
//	if (m_messages.size() > m_messageLimit) {
//		m_messages.pop_front();
//	}
//}
//
//player* LobbyState::getPlayer(unsigned int id) {
//	for (auto playerIt : m_players) {
//		if (playerIt.id == id) {
//			return &playerIt;
//		}
//	}
//	return nullptr;
//}
//
//void LobbyState::addTestData()
//{
//	// Set up players
//	playerJoined("Ollie", m_tempID++);
//	playerJoined("David", m_tempID++);
//	//playerJoined("Press 0 to switch between enter msg and going to gamestate with mouse (MouseClick by default)");
//	//playerJoined("The cause of this is an IMGUI-bug where keyboard focus prevents buttons from working");
//	//playerJoined("This tape is since we'll switch from imgui later anyway, so why patch shit that's only an imgui bug");
//	//playerJoined("If u want to look at it: https://github.com/ocornut/imgui/issues/455#event-1428367449");
//
//	message msg;
//	msg.sender = "Daniel";
//	msg.content = "msg1";
//	m_messages.push_back(msg);
//
//	msg.sender = "Ollie";
//	msg.content = "Message Two";
//	m_messages.push_back(msg);
//
//	msg.sender = "David";
//	msg.content = "msg 3 mr.boss";
//	m_messages.push_back(msg);
//}
//
//void LobbyState::doTestStuff()
//{
//	if (m_input->WasKeyJustPressed(SAIL_KEY_J)) {
//		playerJoined("TestPlayer", m_tempID++);
//	}
//	if (m_input->WasKeyJustPressed(SAIL_KEY_U)) {
//		playerLeft(1);
//	}
//	if (m_input->WasKeyJustPressed(SAIL_KEY_I)) {
//		playerJoined("David", m_tempID++);
//	}
//	if (m_input->WasKeyJustPressed(SAIL_KEY_O)) {
//		playerLeft(m_tempID--);
//	}
//}
//
//void LobbyState::renderPlayerList() {
//	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
//	flags |= ImGuiWindowFlags_NoResize;
//	flags |= ImGuiWindowFlags_NoMove;
//	flags |= ImGuiWindowFlags_NoNav;
//	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
//	flags |= ImGuiWindowFlags_NoTitleBar;
//	flags |= ImGuiWindowFlags_AlwaysAutoResize;
//
//	ImGui::SetNextWindowPos(ImVec2(
//		m_outerPadding,
//		m_outerPadding
//	));
//	ImGui::Begin("Players in lobby:", NULL, flags);
//
//	for (auto currentPlayer : m_players) {
//		ImGui::Text(
//			std::string("- ").append(currentPlayer.name.c_str()).c_str()
//		);
//	}
//	ImGui::End();
//}
//
//void LobbyState::renderStartButton() {
//	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
//	flags |= ImGuiWindowFlags_NoResize;
//	flags |= ImGuiWindowFlags_NoMove;
//	flags |= ImGuiWindowFlags_NoNav;
//	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
//	ImGui::SetNextWindowPos(ImVec2(
//		m_screenWidth - (m_outerPadding + m_screenWidth / 10.0f),
//		m_screenHeight - (m_outerPadding + m_screenHeight / 10.0f)
//	));
//	ImGui::Begin("Press 0 once", NULL, flags);
//
//	// SetKeyBoardFocusHere on the chatbox prevents the button from working,
//	// so if we click with the mouse, temporarily set focus to the button.
//
//	if (ImGui::Button("S.P.L.A.S.H")) {
//		// Queue a removal of LobbyState, then a push of gamestate
//		this->requestStackPop();
//		this->requestStackPush(States::Game);
//	}
//	ImGui::End();
//}
//
//void LobbyState::renderSettings() {
//
//}
//
//void LobbyState::renderChat() {
//	ImGuiWindowFlags chatFlags = ImGuiWindowFlags_NoCollapse;
//	chatFlags |= ImGuiWindowFlags_NoResize;
//	chatFlags |= ImGuiWindowFlags_NoMove;
//	chatFlags |= ImGuiWindowFlags_NoNav;
//	chatFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
//	chatFlags |= ImGuiWindowFlags_NoTitleBar;
//	chatFlags |= ImGuiWindowFlags_AlwaysAutoResize;
//
//	// ------- MESSAGE BOX ------- 
//	ImGui::SetNextWindowPos(ImVec2(
//		m_outerPadding,
//		m_screenHeight - (m_outerPadding + m_textHeight)
//	));
//	ImGui::Begin(
//		"Write Here",
//		NULL,
//		chatFlags
//	);
//
//	if (m_firstFrame) {
//		ImGui::SetKeyboardFocusHere(-1);
//		m_firstFrame = false;
//		m_chatFocus = true;
//	}
//	ImGui::Text("Enter Message:");
//	if (ImGui::InputText("", m_currentMessage, m_messageSizeLimit, ImGuiInputTextFlags_EnterReturnsTrue)) {
//		this->sendMessage(&(string)m_currentMessage);
//		ImGui::SetKeyboardFocusHere(-1);
//		m_chatFocus = true;
//	}
//	ImGui::End();
//
//	// ------- CHAT LOG ------- 
//	ImGui::SetNextWindowSize(ImVec2(
//		400,
//		300
//	));
//	ImGui::SetNextWindowPos(ImVec2(
//		m_outerPadding,
//		m_screenHeight - (300 + m_outerPadding)
//	));
//
//	// Render message history
//	ImGui::Begin("Chat Log", NULL, chatFlags);
//	ImGui::SameLine();
//	ImGui::BeginChild("Messages");
//	for (auto currentMessage : m_messages) {
//		std::string msg;
//		msg.append("HH:MM:SS | ");
//		msg.append(currentMessage.sender);
//		msg.append(" | ");
//		msg.append(currentMessage.content.c_str()).c_str();
//		ImGui::Text(
//			msg.c_str()
//		);
//	}
//	ImGui::EndChild();
//	ImGui::End();
//}
