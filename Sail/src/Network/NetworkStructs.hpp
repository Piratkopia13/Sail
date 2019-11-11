#pragma once
const unsigned int MAX_PACKAGE_SIZE = 64;
const unsigned int MAX_AWAITING_PACKAGES = 1000;
const unsigned int HOST_META_DESC_SIZE = MAX_PACKAGE_SIZE - 6;

// The length of the string you get from archiving an int.
// This is needed to know how many bytes to read to find the size of a message.
constexpr int MSG_SIZE_STR_LEN = 1 + sizeof(int);

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
	struct {
		size_t sizeOfMsg;
		char* rawMsg = nullptr;
	} Message;

	struct
	{
		union {
			ULONG ip_full;
			unsigned char ip_part[4];
		};
		USHORT hostPort;
		char description[HOST_META_DESC_SIZE];
	} HostFoundOnLanData;

	NetworkEventData() {
		Message.rawMsg = nullptr;
		Message.sizeOfMsg = 0;
	}
};
static_assert(sizeof(NetworkEventData) == MAX_PACKAGE_SIZE, "sizeof(NetworkEventData) is not what you expect! Check your struct man.");

struct NetworkEvent {
	NETWORK_EVENT_TYPE eventType;
	TCP_CONNECTION_ID from_tcp_id;
	NetworkEventData* data;

	NetworkEvent() {
		eventType = NETWORK_EVENT_TYPE::NETWORK_ERROR;
		from_tcp_id = 0;
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