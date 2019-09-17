#pragma once
const unsigned int MAX_PACKAGE_SIZE = 64;
const unsigned int MAX_AWAITING_PACKAGES = 1000;
const unsigned int HOST_META_DESC_SIZE = MAX_PACKAGE_SIZE - 6;

//#define DEBUG_NETWORK

typedef unsigned long long TCP_CONNECTION_ID;

enum class NETWORK_EVENT_TYPE {
	NETWORK_ERROR,
	CONNECTION_ESTABLISHED,
	CONNECTION_CLOSED,
	CONNECTION_RE_ESTABLISHED,
	MSG_RECEIVED,
	HOST_ON_LAN_FOUND,
};

union NetworkEventData {
	char rawMsg[MAX_PACKAGE_SIZE];
	struct
	{
		union {
			ULONG ip_full;
			char ip_part[4];
		};
		USHORT hostPort;
		char description[HOST_META_DESC_SIZE];
	} HostFoundOnLanData;
};
static_assert(sizeof(NetworkEventData) == MAX_PACKAGE_SIZE, "sizeof(NetworkEventData) is not what you expect! Check your struct man.");

struct NetworkEvent {
	NETWORK_EVENT_TYPE eventType;
	TCP_CONNECTION_ID clientID;
	NetworkEventData* data;

	NetworkEvent() {
		eventType = NETWORK_EVENT_TYPE::NETWORK_ERROR;
		clientID = 0;
		data = nullptr;
	}
};

class NetworkEventHandler
{
public:
	NetworkEventHandler() {};
	~NetworkEventHandler() {};
	virtual void handleNetworkEvents(NetworkEvent nEvents) = 0;
private:

};