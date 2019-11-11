#include "LobbyState.h"
#include "imgui_internal.h"
//#include "../imgui-sfml-master/imgui-SFML.h"
#include "../libraries/imgui/imgui.h"
#include "../Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "Network/NWrapperSingleton.h"	// New network
#include "Network/NWrapper.h"			// 
#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/Audio/AudioSystem.h"
#include "Sail/entities/components/AudioComponent.h"
#include "Sail/utils/Utils.h"
#include "Sail/utils/SailImGui/SailImGui.h"

#include <string>
#include <list>

LobbyState::LobbyState(StateStack& stack)
	: State(stack),
	m_settingsChanged(false),
	m_renderGameSettings(false),
	m_renderApplicationSettings(false),
	m_timeSinceLastUpdate(0.0f)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
	m_imGuiHandler = m_app->getImGuiHandler();
	m_settings = &m_app->getSettings();

	m_textHeight = 52;
	m_outerPadding = 15;

	m_messageLimit = 14;		// Should be based on size of chatbox instead of fixed
	m_messageCount = 0;
	m_settingBotCount = new int;
	*m_settingBotCount = 0;

	m_messageSizeLimit = 50;
	m_currentmessageIndex = 0;
	m_currentmessage = SAIL_NEW char[m_messageSizeLimit] { 0 };

	m_ready = false;

	m_standaloneButtonflags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBackground;



	// TO DO: Streaming sounds in menu doesn't work because ESC is NOT UPDATED HERE
	//m_lobbyAudio = ECS::Instance()->createEntity("LobbyAudio").get();
	//m_lobbyAudio->addComponent<AudioComponent>();
	//m_lobbyAudio->getComponent<AudioComponent>()->streamSoundRequest_HELPERFUNC("res/sounds/LobbyMusic.xwb", true, true);
}


LobbyState::~LobbyState() {
	delete[] m_currentmessage;
	delete m_settingBotCount;
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

	m_network->checkForPackages();
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

	ImGui::ShowDemoWindow();
	static std::string font = "Beb20";
	//ImGui::PushFont(m_imGuiHandler->getFont(font));
	//
	//if (ImGui::Begin("IMGUISETTINGS")) {
	//	//ImGui::BeginCombo("##FONTS", &font.front());
	//	for (auto const& [key, val] : m_imGuiHandler->getFontMap()) {
	//		ImGui::PushFont(val);
	//
	//		if (ImGui::Selectable(key.c_str(), font == key)) {
	//			font = key;
	//		}
	//		ImGui::PopFont();
	//	}
	//	//ImGui::EndCombo();
	//}
	//ImGui::End();
	//
	//ImGui::PopFont();

	ImGui::PushFont(m_imGuiHandler->getFont(font));
	// ------- menu ----------------
	renderMenu();

	// ------- player LIST ------- 
	renderPlayerList();

	// ------- CHAT LOG ------- 
	renderChat();

	// -------- SETTINGS ----------
	if (m_renderGameSettings) {
		renderGameSettings();
	}


	ImGui::PopFont();

	return false;
}

void LobbyState::addTextToChat(const Message& message) {
	this->addMessageToChat(message);
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
	unsigned char id = stoi(message.sender);
	Player* playa = NWrapperSingleton::getInstance().getPlayer(id);
	std::string msg = playa->name + ": ";
	
	// Work around const
	Message newMessage = message;
	newMessage.content.insert(0, msg);

	// Add message to chatlog
	m_messages.push_back(newMessage);

	// New messages replace old
	if (m_messages.size() > m_messageLimit) {
		m_messages.pop_front();
	}
}

void LobbyState::renderPlayerList() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowPos(ImVec2(
		m_screenWidth - m_outerPadding - 330,
		m_outerPadding
	));
	static ImVec2 windowSize(300, 400);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
	
	static float x[3] = { 0,0,0 };

	if (ImGui::Begin("players in lobby:", NULL, flags)) {

		ImGui::DragFloat3("##ASDASD", &x[0], 0.1f);
		ImGui::Separator();
		ImGui::Text("Player"); 
		ImGui::SameLine(x[0]);

		ImGui::Text("Team"); 
		ImGui::SameLine(x[1]);

		ImGui::Text("Ready"); 
		ImGui::Separator();

		//(windowSize.x) * 0.42f
		std::map<std::string, SettingStorage::Setting>& gamemodeSettings = m_settings->gameSettingsStatic["gamemode"];

		SettingStorage::Setting& selectedGameTeams = m_settings->gameSettingsStatic["Teams"][gamemodeSettings["types"].getSelected().name];
		unsigned char myID = NWrapperSingleton::getInstance().getMyPlayerID();
		for (auto currentplayer : NWrapperSingleton::getInstance().getPlayers()) {
			//PLAYERNAME
			/*if (ImGui::Selectable(std::string("##"+currentplayer.name + std::string((currentplayer.id == myID) ? "*" : "") + std::string("##"+std::to_string(currentplayer.id))).c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
				
			}*/
			
			ImGui::BeginGroup();
			ImGui::Text(std::string(currentplayer.name + std::string((currentplayer.id == myID) ? "*" : "")).c_str());
			ImGui::SameLine(x[0]);
			// TEAM
			if (currentplayer.id == myID || myID == HOST_ID) {
				static unsigned int selectedTeam = 0;
				if (ImGui::BeginCombo("##LABEL", selectedGameTeams.getSelected().name.c_str())) {

					for (auto const& key : selectedGameTeams.options) {
						if (ImGui::Selectable(key.name.c_str(), selectedTeam == (unsigned int)(int)key.value)) {
							// LOCALPLAYER
							if (currentplayer.id == myID) {
								selectedTeam = (unsigned int)(int)key.value;
							}
							//HOST
							else {
								// TODO: SEND NEW SELECTION TO PLAYERS
							}
						}

					}
					ImGui::EndCombo();
				}
			} 
			else {
				//TODO: get team from wrapper
				ImGui::Text("Alone");
			}
			ImGui::SameLine(x[1]);

			// READY 
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			if (currentplayer.id == myID) {
				ImGui::Checkbox(std::string("##Player"+std::to_string(currentplayer.id)).c_str(), &m_ready);
			}
			else {
				bool asd = false;
				ImGui::Checkbox(std::string("##Player" + std::to_string(currentplayer.id)).c_str(), &asd); 
			}
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
			//if (ImGui::BeginPopupContextItem(std::string("item context menu##" + std::to_string(currentplayer.id)).c_str())) {
			//	if (ImGui::Button("KICK")) {
			//		//KICK
			//	}
			//	ImGui::EndPopup();
			//}
			ImGui::EndGroup();
			
			//ImGui::OpenPopupOnItemClick(std::string("item context menu##" + std::to_string(currentplayer.id)).c_str(), 1);

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("First group hovered");
			ImGui::Separator(); 
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

}

void LobbyState::renderGameSettings() {
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
		m_outerPadding + 300,
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
			currentmessage.content.c_str()
		);
	}
	ImGui::EndChild();
	ImGui::End();
}

void LobbyState::renderMenu() {
	static ImVec2 pos(m_outerPadding, m_outerPadding + 100);
	static ImVec2 size(150, 300);
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);

	ImGui::PushFont(m_imGuiHandler->getFont("Beb30"));

	if (ImGui::Begin("##LOBBYMENU", nullptr, m_standaloneButtonflags)) {
	
		if (SailImGui::TextButton("Game Options")) {
			m_renderGameSettings = !m_renderGameSettings;
		}
		if(SailImGui::TextButton("Options")) {
			m_renderApplicationSettings = !m_renderApplicationSettings;

		}
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.3f, 0.3f, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1));
		if(SailImGui::TextButton("Leave")) {

			// Reset the network
			NWrapperSingleton::getInstance().resetNetwork();
			NWrapperSingleton::getInstance().resetWrapper();

			// Schedule new state change
			this->requestStackPop();
			this->requestStackPush(States::MainMenu);
		}
		ImGui::PopStyleColor(2);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.3f, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1));

		if (NWrapperSingleton::getInstance().isHost()) {
			if (SailImGui::TextButton("S.P.L.A.S.H")) {
				// Queue a removal of LobbyState, then a push of gamestate
				NWrapperSingleton::getInstance().stopUDP();
				m_app->getStateStorage().setLobbyToGameData(LobbyToGameData(*m_settingBotCount));
				auto& stat = m_app->getSettings().gameSettingsStatic;
				auto& dynamic = m_app->getSettings().gameSettingsDynamic;
				m_network->sendMsgAllClients({ std::string("i") + m_app->getSettings().serialize(stat, dynamic) });
				m_network->sendMsgAllClients({ std::string("t0") });

				this->requestStackClear();
				this->requestStackPush(States::Game);
			}
		}
		else {
			if (m_ready) {
				if (SailImGui::TextButton("unReady")) {
					// SEND TO HOST THAT PLAYER IS NO LONGER READY
					m_ready = false;
				}
			}
			else {
				if (SailImGui::TextButton("Ready")) {
					// SEND TO HOST THAT PLAYER IS READY
					m_ready = true;
				}
			}

		}
		ImGui::PopStyleColor(2);


	}
	ImGui::End();
	ImGui::PopFont();
}
