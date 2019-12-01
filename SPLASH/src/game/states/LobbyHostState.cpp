#include "LobbyHostState.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperHost.h"
#include "Network/NWrapperSingleton.h"

#include "Sail/events/EventDispatcher.h"

LobbyHostState::LobbyHostState(StateStack& stack)
	: LobbyState(stack) {

	NWrapperSingleton::getInstance().setPlayerID(HOST_ID); 
	NWrapperSingleton::getInstance().startUDP();

	MatchRecordSystem*& recordSystem = NWrapperSingleton::getInstance().recordSystem;
	m_isHost = true;

	if (!recordSystem) {
		recordSystem = new MatchRecordSystem();
		recordSystem->status = 0;
	} else if (recordSystem->status == 1) {
		recordSystem = new MatchRecordSystem();
		recordSystem->initReplay();
	} else {
		delete recordSystem;
		recordSystem = nullptr;
	}


}

LobbyHostState::~LobbyHostState() {

}
