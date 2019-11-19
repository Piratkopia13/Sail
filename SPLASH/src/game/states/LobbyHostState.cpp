#include "LobbyHostState.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperHost.h"
#include "Network/NWrapperSingleton.h"

#include "Sail/events/EventDispatcher.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack) {

	NWrapperSingleton::getInstance().setPlayerID(HOST_ID); 
	NWrapperSingleton::getInstance().startUDP();	
}

LobbyHostState::~LobbyHostState() {

}

bool LobbyHostState::onMyTextInput(const ChatSent& event) {
		Message temp{ NWrapperSingleton::getInstance().getMyPlayer().id, m_message };
		addMessageToChat(temp);
		m_network->sendChatMsg(m_message);


	return true;
}