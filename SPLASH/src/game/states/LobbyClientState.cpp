#include "LobbyClientState.h"

#include "Sail/events/EventDispatcher.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperClient.h"
#include "Network/NWrapperSingleton.h"

LobbyClientState::LobbyClientState(StateStack& stack)
	: LobbyState(stack),
	m_wasDropped(false) {

	EventDispatcher::Instance().subscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DROPPED, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_START_GAME, this);
	EventDispatcher::Instance().subscribe(Event::Type::SETTINGS_UPDATED, this);
}

LobbyClientState::~LobbyClientState() {
	if (m_wasDropped) {
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();
	}

	EventDispatcher::Instance().unsubscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DROPPED, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_START_GAME, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::SETTINGS_UPDATED, this);
}

bool LobbyClientState::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::TEXTINPUT:			onMyTextInput((const TextInputEvent&)event); break;
	case Event::Type::NETWORK_CHAT:			onRecievedText((const NetworkChatEvent&)event); break;
	case Event::Type::NETWORK_JOINED:		onPlayerJoined((const NetworkJoinedEvent&)event); break;
	case Event::Type::NETWORK_DISCONNECT:	onPlayerDisconnected((const NetworkDisconnectEvent&)event); break;
	case Event::Type::NETWORK_DROPPED:		onDropped((const NetworkDroppedEvent&)event); break;
	case Event::Type::NETWORK_START_GAME:	onStartGame((const NetworkStartGameEvent&)event); break;
	case Event::Type::SETTINGS_UPDATED:		onSettingsChanged((const SettingsUpdatedEvent&)event); break;
	default: break;
	}
	return true;
}

bool LobbyClientState::onMyTextInput(const TextInputEvent& event) {
	// Add input to current message, If 'enter', send message to host, do not input to chat.
	if (this->inputToChatLog(event.msg)) {
		std::string mesgWithId = "";
		mesgWithId += std::to_string(NWrapperSingleton::getInstance().getMyPlayerID()) + ':';
		mesgWithId += m_currentmessage;
		std::string msg = this->fetchMessage();
		m_network->sendChatMsg(mesgWithId);
	}
	
	return false;
}

bool LobbyClientState::onRecievedText(const NetworkChatEvent& event) {
	// Only add the received message to the chat
	this->addTextToChat(event.chatMessage);

	return false;
}

bool LobbyClientState::onPlayerJoined(const NetworkJoinedEvent& event) {
	return true;
}

bool LobbyClientState::onPlayerDisconnected(const NetworkDisconnectEvent& event) {
	// Remove the player from the player list
	unsigned char id = event.player_id;
	NWrapperSingleton::getInstance().playerLeft(id);

	return false;
}

bool LobbyClientState::onDropped(const NetworkDroppedEvent& event) {
	// Queue changes to statestack
	this->requestStackPop();
	this->requestStackPush(States::MainMenu);

	// Reset network so that user can choose host/client again.
	m_wasDropped = true;


	return false;
}

bool LobbyClientState::onStartGame(const NetworkStartGameEvent& event) {
	// Queue changes to the stack while maintaining the connection

	m_app->getStateStorage().setLobbyToGameData(LobbyToGameData(*m_settingBotCount));
	this->requestStackClear();
	this->requestStackPush(States::Game);

	return true;
}

bool LobbyClientState::onSettingsChanged(const SettingsUpdatedEvent& event) {
	auto& stat = m_app->getSettings().gameSettingsStatic;
	auto& dynamic = m_app->getSettings().gameSettingsDynamic;
	m_app->getSettings().deSerialize(event.settings, stat, dynamic);
	return false;
}
