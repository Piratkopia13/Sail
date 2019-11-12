#include "LobbyState.h"


#include "../libraries/imgui/imgui.h"
#include "../Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"

#include "Network/NWrapperSingleton.h"
#include "Network/NWrapper.h"
#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/Audio/AudioSystem.h"
#include "Sail/entities/components/AudioComponent.h"
#include "Sail/utils/Utils.h"

LobbyState::LobbyState(StateStack& stack)
	: State(stack),
	m_settingsChanged(false),
	m_timeSinceLastUpdate(0.0f)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
	m_textHeight = 52;
	m_outerPadding = 15;

	m_messageLimit = 14;		// Should be based on size of chatbox instead of fixed
	m_messageCount = 0;
	m_settingBotCount = new int;
	*m_settingBotCount = 0;

	m_messageSizeLimit = 50;
	m_currentmessageIndex = 0;
	m_currentmessage = SAIL_NEW char[m_messageSizeLimit] { 0 };
	m_spectator = false;

	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().subscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
}


LobbyState::~LobbyState() {
	delete[] m_currentmessage;
	delete m_settingBotCount;

	EventDispatcher::Instance().unsubscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
}

bool LobbyState::processInput(float dt) {
	if (m_input->IsMouseButtonPressed(0)) {
		m_chatFocus = false;
	}

	return false;
}

bool LobbyState::inputToChatLog(const MSG& msg) {
	if (m_currentmessageIndex < m_messageSizeLimit && msg.wParam != KeyBinds::SEND_MESSAGE) {
		// Add whichever button that was inputted to the current message
		// --- OBS : doesn't account for capslock, etc.
		m_currentmessage[m_currentmessageIndex++] = (char)msg.wParam;
	}
	if (msg.wParam == KeyBinds::SEND_MESSAGE && m_chatFocus == false) {
		return true;
	}
	return false;
}

bool LobbyState::update(float dt, float alpha) {
	// Update screen dimensions & ImGui related
	// (Sure, events, but the only thing consuming resources is the LobbyState)
	this->m_screenWidth = m_app->getWindow()->getWindowWidth();
	this->m_screenHeight = m_app->getWindow()->getWindowHeight();


	NWrapperSingleton::getInstance().checkForPackages();

	if (NWrapperSingleton::getInstance().isHost() && m_settingsChanged && m_timeSinceLastUpdate > 0.2f) {
		auto& stat = m_app->getSettings().gameSettingsStatic;
		auto& dynamic = m_app->getSettings().gameSettingsDynamic;
		m_network->sendMsgAllClients({ std::string("i") + m_app->getSettings().serialize(stat, dynamic) });
		m_settingsChanged = false;
		m_timeSinceLastUpdate = 0.0f;
	}
	m_timeSinceLastUpdate += dt;

	return false;
}

bool LobbyState::render(float dt, float alpha) {
	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	ECS::Instance()->getSystem<BeginEndFrameSystem>()->renderNothing();
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

	// ------- Return BUTTON ------- 
	renderQuitButton();

	// ------- Spectator toggle box ------- 
	renderSpectatorButton();

	return false;
}

bool LobbyState::onEvent(const Event& event) {

	switch (event.type) {

	case Event::Type::TEXTINPUT:			onMyTextInput((const TextInputEvent&)event); break;
	case Event::Type::NETWORK_CHAT:			onRecievedText((const NetworkChatEvent&)event); break;
	case Event::Type::NETWORK_JOINED:		onPlayerJoined((const NetworkJoinedEvent&)event); break;
	case Event::Type::NETWORK_DISCONNECT:	onPlayerDisconnected((const NetworkDisconnectEvent&)event); break;

	default:
		break;
	}

	return true;
}

void LobbyState::resetCurrentMessage() {
	m_currentmessageIndex = 0;
	for (size_t i = 0; i < m_messageSizeLimit; i++) {
		m_currentmessage[i] = '\0';
	}
}

std::string LobbyState::fetchMessage()
{
	std::string message = std::string(m_currentmessage);

	// Reset currentMessage
	m_currentmessageIndex = 0;
	for (size_t i = 0; i < m_messageSizeLimit; i++) {
		m_currentmessage[i] = '\0';
	}

	return message;
}

void LobbyState::addMessageToChat(const Message& message) {
	// Replace '0: Blah blah message' --> 'Daniel: Blah blah message'
	// Add sender to the text

	std::string msg;
	if (message.senderID != 255) {
		Player* playa = NWrapperSingleton::getInstance().getPlayer(message.senderID);
		msg = playa->name + ": ";
	}
	msg += message.content;
	// Add message to chatlog
	m_messages.push_back(msg);

	// New messages replace old
	if (m_messages.size() > m_messageLimit) {
		m_messages.pop_front();
	}
}

bool LobbyState::onRecievedText(const NetworkChatEvent& event) {
	addMessageToChat(event.chatMessage);
	return true;
}

bool LobbyState::onPlayerJoined(const NetworkJoinedEvent& event) {
	Message message;
	message.content = event.player.name + " joined the game!";
	message.senderID = 255;
	addMessageToChat(message);
	return true;
}

bool LobbyState::onPlayerDisconnected(const NetworkDisconnectEvent& event) {
	Message message;
	message.content = event.player.name + " left the game!";
	message.senderID = 255;
	addMessageToChat(message);
	return true;
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

	unsigned char myID = NWrapperSingleton::getInstance().getMyPlayerID();
	for (auto currentplayer : NWrapperSingleton::getInstance().getPlayers()) {
		std::string temp;
		temp += " - ";
		temp += currentplayer.name.c_str();

		if (currentplayer.id == myID) {
			temp += " (You)";
		}
		
		ImGui::Text(temp.c_str());
	}
	ImGui::End();
}

void LobbyState::renderStartButton() {
	if (NWrapperSingleton::getInstance().isHost()) {
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
		ImGui::Begin("Start Game");

		if (ImGui::Button("S.P.L.A.S.H")) {
			// Queue a removal of LobbyState, then a push of gamestate
			NWrapperSingleton::getInstance().stopUDP();
			if (m_spectator) {
				m_app->getStateStorage().setLobbyToGameData(LobbyToGameData(*m_settingBotCount, true));
			}
			else {
				m_app->getStateStorage().setLobbyToGameData(LobbyToGameData(*m_settingBotCount, false));
			}
			auto& stat = m_app->getSettings().gameSettingsStatic;
			auto& dynamic = m_app->getSettings().gameSettingsDynamic;
			m_network->sendMsgAllClients({ std::string("i") + m_app->getSettings().serialize(stat, dynamic)});
			m_network->sendMsgAllClients({ std::string("tp0")});
			
			this->requestStackClear();
			this->requestStackPush(States::Game);
		}
		ImGui::End();
	}
}

void LobbyState::renderQuitButton() {
	ImGui::Begin("Return");
	if (ImGui::Button("Quit to Main Menu")) {
	
		// Set temp window pos
		ImGui::SetWindowPos({ 424,641 });
		ImGui::SetWindowSize({ 137,63 });

		// Reset the network
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();

		// Schedule new state change
		this->requestStackPop();
		this->requestStackPush(States::MainMenu);
	}
	ImGui::End();
}

void LobbyState::renderSettings() {
	ImGuiWindowFlags settingsFlags = ImGuiWindowFlags_NoCollapse;
	settingsFlags |= ImGuiWindowFlags_NoResize;
	settingsFlags |= ImGuiWindowFlags_NoMove;
	settingsFlags |= ImGuiWindowFlags_NoNav;
	settingsFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	settingsFlags |= ImGuiWindowFlags_NoTitleBar;
	settingsFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	settingsFlags |= ImGuiWindowFlags_NoSavedSettings;


	// Uncomment when we actually have game settings
	ImGui::SetNextWindowPos(ImVec2(
		m_screenWidth - m_outerPadding - 330,
		m_outerPadding
	));

	ImGui::SetNextWindowSize(ImVec2(300,300));
	if (ImGui::Begin("Settings", NULL, settingsFlags)) {
		static auto& dynamic = m_app->getSettings().gameSettingsDynamic;
		static auto& stat = m_app->getSettings().gameSettingsStatic;
		ImGui::Text("Map Settings (not doing anything yet..)");
		ImGui::Separator();
		ImGui::Columns(2);
		ImGui::Text("Setting"); ImGui::NextColumn();
		ImGui::Text("Value"); ImGui::NextColumn();
		ImGui::Separator();

		SettingStorage::DynamicSetting* mapSizeX = &m_app->getSettings().gameSettingsDynamic["map"]["sizeX"];
		SettingStorage::DynamicSetting* mapSizeY = &m_app->getSettings().gameSettingsDynamic["map"]["sizeY"];

		static int size[] = { 0,0 };
		size[0] = (int)mapSizeX->value;
		size[1] = (int)mapSizeY->value;
		ImGui::Text("MapSize"); ImGui::NextColumn();
		if (ImGui::SliderInt2("##MapSizeXY", size, (int)mapSizeX->minVal, (int)mapSizeX->maxVal)){
			mapSizeX->value = size[0];
			mapSizeY->value = size[1];
			m_settingsChanged = true;
		}
		ImGui::NextColumn();

		int seed = dynamic["map"]["seed"].value;
		ImGui::Text("Seed"); ImGui::NextColumn();
		if (ImGui::InputInt("##SEED", &seed)) {
			dynamic["map"]["seed"].setValue(seed);
			m_settingsChanged = true;
		}
		ImGui::NextColumn();
		ImGui::Text("Clutter"); ImGui::NextColumn();
		float val = m_app->getSettings().gameSettingsDynamic["map"]["clutter"].value;
		if (ImGui::SliderFloat("##Clutter", 
			&val,
			m_app->getSettings().gameSettingsDynamic["map"]["clutter"].minVal,
			m_app->getSettings().gameSettingsDynamic["map"]["clutter"].maxVal
		)) {
			m_app->getSettings().gameSettingsDynamic["map"]["clutter"].setValue(val);
			m_settingsChanged = true;
		}
		ImGui::NextColumn();

		ImGui::Columns(1);
	}
	ImGui::End();
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
	chatFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	chatFlags |= ImGuiWindowFlags_NoInputs;

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
		m_firstFrame = false;
		m_chatFocus = false;
	}
	ImGui::Text("Enter message:");
	if (ImGui::InputText("", m_currentmessage, m_messageSizeLimit, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_EnterReturnsTrue)) {
		m_chatFocus = false;
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
			currentmessage.c_str()
		);
	}
	ImGui::EndChild();
	ImGui::End();
}

void LobbyState::renderSpectatorButton() {
	ImGui::Begin("Spectator");
	//ImGui::SetWindowPos({ 1018, 633 });
	ImGui::SetWindowSize({ 112, 57 });
	ImGui::Checkbox("", &m_spectator);
	ImGui::End();
}


