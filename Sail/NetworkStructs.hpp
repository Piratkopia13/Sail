#pragma once
#define MAX_PACKAGE_SIZE 64
#define MAX_AWAITING_PACKAGES 1000
//#define DEBUG_NETWORK

typedef unsigned long long ConnectionID;

enum class NETWORK_EVENT_TYPE {
	NETWORK_ERROR,
	CLIENT_JOINED,
	CLIENT_DISCONNECTED,
	CLIENT_RECONNECTED,
	MSG_RECEIVED,
};

struct MessageData {
	char msg[MAX_PACKAGE_SIZE];
};

struct NetworkEvent {
	NETWORK_EVENT_TYPE eventType;
	ConnectionID clientID;
	MessageData* data;

	NetworkEvent() {
		eventType = NETWORK_EVENT_TYPE::NETWORK_ERROR;
		clientID = 0;
		data = nullptr;
	}
};