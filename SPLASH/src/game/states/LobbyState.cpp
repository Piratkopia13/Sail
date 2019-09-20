#include "LobbyState.h"


//#include "../imgui-sfml-master/imgui-SFML.h"
#include "../libraries/imgui/imgui.h"
#include "../Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "Network/NWrapperSingleton.h"	// New network
#include "Network/NWrapper.h"			// 


#include <string>
using namespace std;

LobbyState::LobbyState(StateStack& stack)
	: State(stack)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
	m_textHeight = 52;
	m_outerPadding = 15;

	m_messageLimit = 14;		// Should be based on size of chatbox instead of fixed
	m_playerLimit = 12;
	m_playerCount = 0;
	m_messageCount = 0;

	// Set name according to data from menustate
	m_me.name = m_app->getStateStorage().getMenuToLobbyData()->name;

	m_messageSizeLimit = 50;
	m_currentmessageIndex = 0;
	m_currentmessage = SAIL_NEW char[m_messageSizeLimit] { 0 };
}

LobbyState::~LobbyState() {
	delete[]m_currentmessage;
}

bool LobbyState::processInput(float dt) {
	if (m_input->IsMouseButtonPressed(0)) {
		m_chatFocus = false;
	}

	return false;
}

bool LobbyState::inputToChatLog(MSG& msg) {
	if (m_currentmessageIndex < m_messageSizeLimit && msg.wParam != SAIL_KEY_RETURN) {
		// Add whichever button that was inputted to the current message
		// --- OBS : doesn't account for capslock, etc.
		m_currentmessage[m_currentmessageIndex++] = (char)msg.wParam;
	}
	if (msg.wParam == SAIL_KEY_RETURN && m_chatFocus == false) {
		return true;
	}
	return false;
}

void LobbyState::resetPlayerList()
{
	m_players.clear();
	m_playerCount = 0;
}

bool LobbyState::update(float dt) {
	// Update screen dimensions & ImGui related
	// (Sure, events, but the only thing consuming resources is the LobbyState)
	this->m_screenWidth = m_app->getWindow()->getWindowWidth();
	this->m_screenHeight = m_app->getWindow()->getWindowHeight();

	m_network->checkForPackages();

	return false;
}

bool LobbyState::render(float dt, float alpha) {
	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	m_scene.draw();
	return false;
}

bool LobbyState::renderImgui(float dt) {
	// ------- player LIST ------- 
	renderPlayerList();

	// ------- CHAT LOG ------- 
	renderChat();

	// -------- SETTINGS ----------
	renderSettings();

	// ------- START BUTTON ------- 
	renderStartButton();

	return false;
}

bool LobbyState::playerJoined(Player& player) {
	if (m_playerCount < m_playerLimit) {
		m_players.push_back(player);
		m_playerCount++;
		return true;
	}
	return false;
}

bool LobbyState::playerLeft(unsigned char& id) {
	// Linear search to get target 'player' struct, then erase that from the list
	Player* toBeRemoved = nullptr;
	int pos = 0;
	for (auto playerIt : m_players) {
		if (playerIt.id == id) {
			toBeRemoved = &playerIt;
			m_players.remove(*toBeRemoved);
			return true;
		}
	}

	return false;
}

void LobbyState::addTextToChat(Message* message) {
	this->addMessageToChat(*message);
}

void LobbyState::resetCurrentMessage() {
	m_currentmessageIndex = 0;
	for (size_t i = 0; i < m_messageSizeLimit; i++) {
		m_currentmessage[i] = '\0';
	}
}

string LobbyState::fetchMessage()
{
	string message = string(m_currentmessage);

	// Reset currentMessage
	m_currentmessageIndex = 0;
	for (size_t i = 0; i < m_messageSizeLimit; i++) {
		m_currentmessage[i] = '\0';
	}

	return message;
}

void LobbyState::addMessageToChat(Message& message) {
	// Replace '0: Blah blah message' --> 'Daniel: Blah blah message'
	// Add sender to the text
	unsigned char id = stoi(message.sender);
	Player* playa = this->getPlayer(id);
	string msg = playa->name + ": ";
	message.content.insert(0, msg);

	// Add message to chatlog
	m_messages.push_back(message);

	// New messages replace old
	if (m_messages.size() > m_messageLimit) {
		m_messages.pop_front();
	}
}

Player* LobbyState::getPlayer(unsigned char& id) {
	Player* foundPlayer = nullptr;
	for (Player& player : m_players) {
		if (player.id == id) {
			foundPlayer = &player;
			break;
			//return foundPlayer;
		}
	}
	
	return foundPlayer;
}

void LobbyState::renderPlayerList() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;
	flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_outerPadding
	));
	ImGui::Begin("players in lobby:", NULL, flags);

	for (auto currentplayer : m_players) {
		std::string temp;
		temp += " - ";
		temp += currentplayer.name.c_str();

		if (currentplayer.id == m_me.id) {
			temp += " (You)";
		}
		
		ImGui::Text(temp.c_str());
	}
	ImGui::End();
}

void LobbyState::renderStartButton() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoSavedSettings;
	ImGui::SetNextWindowPos(ImVec2(
		m_screenWidth - (m_outerPadding + m_screenWidth / 10.0f),
		m_screenHeight - (m_outerPadding + m_screenHeight / 10.0f)
	));
	ImGui::Begin("Press 0 once");

	// SetKeyBoardFocusHere on the chatbox prevents the button from working,
	// so if we click with the mouse, temporarily set focus to the button.

	if (ImGui::Button("S.P.L.A.S.H")) {
		// Queue a removal of LobbyState, then a push of gamestate
		this->requestStackPop();
		this->requestStackPush(States::Game);
	}
	ImGui::End();
}

void LobbyState::renderSettings() {

}

void LobbyState::renderChat() {
	ImGuiWindowFlags chatFlags = ImGuiWindowFlags_NoCollapse;
	chatFlags |= ImGuiWindowFlags_NoResize;
	chatFlags |= ImGuiWindowFlags_NoMove;
	chatFlags |= ImGuiWindowFlags_NoNav;
	chatFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	chatFlags |= ImGuiWindowFlags_NoTitleBar;
	chatFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	chatFlags |= ImGuiWindowFlags_NoSavedSettings;

	// ------- message BOX ------- 
	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (m_outerPadding + m_textHeight)
	));
	ImGui::Begin(
		"Write Here",
		NULL,
		chatFlags
	);

	if (m_firstFrame) {
		ImGui::SetKeyboardFocusHere(-1);
		m_firstFrame = false;
		m_chatFocus = true;
	}
	ImGui::Text("Enter message:");
	if (ImGui::InputText("", m_currentmessage, m_messageSizeLimit, ImGuiInputTextFlags_EnterReturnsTrue)) {
		//this->sendmessage(&(string)m_currentmessage);
		ImGui::SetKeyboardFocusHere(-1);
		m_chatFocus = true;
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
	ImGui::BeginChild("messages");
	for (auto currentmessage : m_messages) {
		ImGui::Text(
			currentmessage.content.c_str()
		);
	}
	ImGui::EndChild();
	ImGui::End();
}