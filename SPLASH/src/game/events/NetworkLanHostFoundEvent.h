#pragma once



#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkLanHostFoundEvent : public Event {
public:
	NetworkLanHostFoundEvent(ULONG lobbyInfo, USHORT hostPort) : Event(Event::NETWORK_LAN_HOST_FOUND) {
		m_ip = lobbyInfo;
		m_hostPort = hostPort;
	}
	~NetworkLanHostFoundEvent() {}

	static Type getStaticType() {
		return Event::NETWORK_LAN_HOST_FOUND;
	}

	ULONG getIp() { return m_ip; }
	USHORT getHostPort() { return m_hostPort; }

private:
	ULONG m_ip = 0;
	USHORT m_hostPort;

};