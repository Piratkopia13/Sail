#include "NWrapper.h"

#include "NWrapperClient.h"
#include "NWrapperHost.h"


void NWrapper::initAsHost() {
	isHost = true;
}

void NWrapper::initAsClient() {
	isHost = false;
}

NWrapper* NWrapper::getInstance() {
	static NWrapper* instance;
	if (isInitialized == false) {
		isInitialized = true;
		
		if (isHost == true) {
			instance = new NWrapperHost;
		}
		else {
			instance = new NWrapperClient;
		}
	}

	return instance;
}

void NWrapper::handleNetworkEvents(NetworkEvent nEvent) {
	switch (nEvent.eventType) {
	case NETWORK_EVENT_TYPE::NETWORK_ERROR:
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_ESTABLISHED:
		playerJoined(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_CLOSED:
		playerDisconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_RE_ESTABLISHED:
		playerReconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::MSG_RECEIVED:
		decodeMessage(nEvent);
		break;
	default:
		break;
	}
}
