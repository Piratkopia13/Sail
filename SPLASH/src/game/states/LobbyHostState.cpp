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



	return true;
}