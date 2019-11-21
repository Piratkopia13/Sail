#include "pch.h"
#include "ChatWindow.h"
#include "imgui.h"
#include "Sail/Application.h"
#include "Network/NWrapperSingleton.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "Sail/events/types/NetworkPlayerChangedTeam.h"

ChatWindow::ChatWindow(bool showWindow) :
	m_message(""),
	m_timeSinceLastMessage(0.0f),
	m_fadeTime(-1.0f),
	m_fadeThreshold(-1.0f),
	m_scrollToBottom(false),
	m_messageLimit(30),
	m_messageSizeLimit(30)
{
	m_app = Application::getInstance();
	m_settings = &m_app->getSettings();
	m_imguiHandler = m_app->getImGuiHandler();
	m_eventDispatcher = &EventDispatcher::Instance();
	m_network = &NWrapperSingleton::getInstance();


	m_eventDispatcher->subscribe(Event::Type::CHATSENT, this);
	m_eventDispatcher->subscribe(Event::Type::NETWORK_CHAT, this);
	m_eventDispatcher->subscribe(Event::Type::NETWORK_JOINED, this);
	m_eventDispatcher->subscribe(Event::Type::NETWORK_DISCONNECT, this);
	m_eventDispatcher->subscribe(Event::Type::NETWORK_PLAYER_CHANGED_TEAM, this);





	m_standaloneButtonflags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBackground;
	m_backgroundOnlyflags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoSavedSettings;


}

ChatWindow::~ChatWindow() {
	m_eventDispatcher->unsubscribe(Event::Type::CHATSENT, this);
	m_eventDispatcher->unsubscribe(Event::Type::NETWORK_CHAT, this);
	m_eventDispatcher->unsubscribe(Event::Type::NETWORK_JOINED, this);
	m_eventDispatcher->unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
	m_eventDispatcher->unsubscribe(Event::Type::NETWORK_PLAYER_CHANGED_TEAM, this);
}

bool ChatWindow::onEvent(const Event& event) {


	switch (event.type) {
				
	case Event::Type::CHATSENT:						onMyTextInput((const ChatSent&)event); break;
	case Event::Type::NETWORK_CHAT:					onRecievedText((const NetworkChatEvent&)event); break;
	case Event::Type::NETWORK_JOINED:				onPlayerJoined((const NetworkJoinedEvent&)event); break;
	case Event::Type::NETWORK_DISCONNECT:			onPlayerDisconnected((const NetworkDisconnectEvent&)event); break;
	case Event::Type::NETWORK_PLAYER_CHANGED_TEAM:	onPlayerTeamChanged((const NetworkPlayerChangedTeam&)event); break;

	default:
		break;
	}

	return true;
}



void ChatWindow::renderWindow() {




}

void ChatWindow::renderChat(float dt) {
	m_timeSinceLastMessage += dt;

	



	ImGuiWindowFlags chatFlags = ImGuiWindowFlags_NoCollapse;
	chatFlags |= ImGuiWindowFlags_NoResize;
	chatFlags |= ImGuiWindowFlags_NoMove;
	chatFlags |= ImGuiWindowFlags_NoNav;
	chatFlags |= ImGuiWindowFlags_NoTitleBar;
	chatFlags |= ImGuiWindowFlags_NoSavedSettings;


	ImGui::SetNextWindowPos(m_position);
	ImGui::SetNextWindowSize(m_size);
	//ImGui::SetNextWindowSizeConstraints(size); Keep
	static bool focus = false;
	static bool pop = false;
	pop = false;
	if (focus) {
		m_timeSinceLastMessage = 0.0f;
	}
	static float alpha = 1.0f;
	alpha = 1.0f;
	if (m_timeSinceLastMessage > m_fadeThreshold && m_fadeThreshold >= 0.0f) {
		pop = true;
		alpha = 1.0f - ((m_timeSinceLastMessage - m_fadeThreshold) / m_fadeTime);
		alpha = alpha < 0.02f ? 0.02f : alpha;
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

	}


	if (ImGui::Begin("##CHATWINDOW", nullptr, chatFlags)) {
		ImGui::PushFont(m_imguiHandler->getFont("Beb20"));
		if (ImGui::BeginChild("##CHATTEXT", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10))) {
			for (auto currentMessage : m_messages) {
				//System or player which has left
				if (currentMessage.id == 255 || currentMessage.id == 254) {
					ImGui::Text(currentMessage.message.c_str());
				}
				//Player
				else {
					if (auto * player = NWrapperSingleton::getInstance().getPlayer(currentMessage.id)) {
						int team = player->team;
						glm::vec3 temp = m_settings->getColor(m_settings->teamColorIndex(team));
						ImVec4 col(
							temp.r,
							temp.g,
							temp.b,
							1
						);
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
						ImGui::PushStyleColor(ImGuiCol_Text, col);
						ImGui::Text(player->name.c_str());
						ImGui::PopStyleColor();
						ImGui::SameLine();
						ImGui::Text(std::string(": " + currentMessage.message).c_str());
						ImGui::PopStyleVar();
					}


				}


			}
			if (m_scrollToBottom) {
				ImGui::SetScrollHereY(1.0f);
				m_scrollToBottom = false;
			}
		}
		ImGui::EndChild();
		ImGui::PopFont();

		static bool justSent = false;
		static bool releasedEnter = true;
		justSent = false;
		static char buf[101] = "";
		strncpy_s(buf, m_message.c_str(), m_message.size());
		if (ImGui::IsKeyPressed(SAIL_KEY_RETURN, false)) {
			focus = true;
			ImGui::SetKeyboardFocusHere(-1);
			justSent = true;

		}
		if (ImGui::IsKeyReleased(SAIL_KEY_RETURN)) {
			releasedEnter = true;
		}
		ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.9f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
		if (ImGui::InputTextWithHint("##ChatInput", (focus ? "" : "Press enter to chat"), buf, m_messageSizeLimit, ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_message = buf;
			if (m_message != "" && releasedEnter) {
				//Send Message
				EventDispatcher::Instance().emit(ChatSent(m_message));
				m_message = "";
			}
			releasedEnter = false;
			justSent = true;
			ImGui::SetKeyboardFocusHere(0);

		}
		else {
			m_message = buf;
		}
		ImGui::PopStyleVar();
		focus = ImGui::IsItemActive() || justSent;
		ImGui::SameLine();
		if (ImGui::Button(" >  ")) {
			if (m_message != "") {
				EventDispatcher::Instance().emit(ChatSent(m_message));
				m_message = "";
				justSent = true;
			}
		}


	}
	if (pop) {
		ImGui::PopStyleVar();
	}


	ImGui::End();
}

void ChatWindow::addSystemMessage(const std::string& message) {
	
}

void ChatWindow::addMessage(const int id, const std::string& name, const std::string message) {
	m_timeSinceLastMessage = 0.0f;
	m_messages.push_back({ id, name, message });
	if (m_messages.size() > m_messageLimit) {
		m_messages.pop_front();
	}

	m_scrollToBottom = true;
}

void ChatWindow::clearHistory() {
	m_messages.clear();
}


bool ChatWindow::onMyTextInput(const ChatSent& event) {

	if (m_network->isHost()) {
		addMessage(
			NWrapperSingleton::getInstance().getMyPlayer().id,
			NWrapperSingleton::getInstance().getMyPlayer().name,
			event.msg
		);
	}
	m_network->getNetworkWrapper()->sendChatMsg(event.msg);


	return false;
}

bool ChatWindow::onRecievedText(const NetworkChatEvent& event) {
	addMessage(event.chatMessage.senderID, m_network->getPlayer(event.chatMessage.senderID)->name, event.chatMessage.content);
	return true;
}


bool ChatWindow::onPlayerTeamChanged(const NetworkPlayerChangedTeam& event) {
	auto* settings = &m_app->getSettings();
	std::unordered_map<std::string, SettingStorage::Setting>& gamemodeSettings = settings->gameSettingsStatic["gamemode"];
	SettingStorage::Setting& selectedGameTeams = settings->gameSettingsStatic["Teams"][gamemodeSettings["types"].getSelected().name];

	Player* p = NWrapperSingleton::getInstance().getPlayer(event.playerID);

	int currentlySelected = 0;
	for (auto t : selectedGameTeams.options) {
		if ((int)(t.value) == (int)(p->team)) {
			break;
		}
		currentlySelected++;
	}
	
	std::string content = NWrapperSingleton::getInstance().getMyPlayerID() == event.playerID ? "You" : NWrapperSingleton::getInstance().getPlayer(event.playerID)->name;
	content += " changed team to " + selectedGameTeams.options[currentlySelected > 0].name;
	addMessage(255, "", content);
	return true;
}

bool ChatWindow::onPlayerJoined(const NetworkJoinedEvent& event) {

	addMessage(255, "", event.player.name + " joined the game!");
	return true;
}



bool ChatWindow::onPlayerDisconnected(const NetworkDisconnectEvent& event) {

	auto it = m_messages.begin();
	while (it != m_messages.end()) {
		if (it->id == event.player.id) {
			it->id = char(254);
		}
		it++;
	}

	addMessage(255, "", event.player.name + (event.reason == PlayerLeftReason::KICKED ? " was kicked!" : " left the game!"));
	return true;
}