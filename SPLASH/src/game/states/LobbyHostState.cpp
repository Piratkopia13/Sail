#include "LobbyHostState.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperHost.h"
#include "Sail/events/EventDispatcher.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack) {
	// Reserved for host, all other will get 1,2,3,...,n
	NWrapperSingleton::getInstance().setPlayerID(HOST_ID); 
	NWrapperSingleton::getInstance().startUDP();
	
	//NWrapperHost* wrapper = static_cast<NWrapperHost*>(NWrapperSingleton::getInstance().getNetworkWrapper());
	//wrapper->setLobbyName(NWrapperSingleton::getInstance().getMyPlayer().name.c_str());

	EventDispatcher::Instance().subscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DISCONNECT, this);
}

LobbyHostState::~LobbyHostState() {
	EventDispatcher::Instance().unsubscribe(Event::Type::TEXTINPUT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_CHAT, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_JOINED, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DISCONNECT, this);
}

bool LobbyHostState::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::TEXTINPUT:			onMyTextInput((const TextInputEvent&)event); break;
	case Event::Type::NETWORK_CHAT:			onRecievedText((const NetworkChatEvent&)event); break;
	case Event::Type::NETWORK_JOINED:		onPlayerJoined((const NetworkJoinedEvent&)event); break;
	case Event::Type::NETWORK_DISCONNECT:	onPlayerDisconnected((const NetworkDisconnectEvent&)event); break;
	default: break;
	}

	return false;
}

bool LobbyHostState::onMyTextInput(const TextInputEvent& event) {
	// Add to current message, If 'enter' ...
	if (this->inputToChatLog(event.msg)) {
		// ... Add current message to chat log
		Message temp{ std::to_string(NWrapperSingleton::getInstance().getMyPlayer().id), m_currentmessage };
		this->addTextToChat(temp);

		// ... Append my ID to it.
		std::string mesgWithId = "";
		mesgWithId += std::to_string(NWrapperSingleton::getInstance().getMyPlayer().id) + ':';
		mesgWithId += m_currentmessage;
		std::string msg = this->fetchMessage();

		// ... Send the message to other clients and reset message
		m_network->sendChatAllClients(mesgWithId);
	}

	return true;
}

bool LobbyHostState::onRecievedText(const NetworkChatEvent& event) {
	// Add received text to chat log
	this->addTextToChat(event.chatMessage);

	return true;
}

bool LobbyHostState::onPlayerJoined(const NetworkJoinedEvent& event) {
	return true;
}

bool LobbyHostState::onPlayerDisconnected(const NetworkDisconnectEvent& event) {
	return true;
}