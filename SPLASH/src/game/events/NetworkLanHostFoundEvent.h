#pragma once



#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkLanHostFoundEvent : public Event {
public:
	NetworkLanHostFoundEvent(ULONG lobbyInfo, USHORT hostPort, std::string desc) : Event(Event::NETWORK_LAN_HOST_FOUND) {
		m_ip = lobbyInfo;
		m_hostPort = hostPort;
		m_description = desc;
	}
	~NetworkLanHostFoundEvent() {}

	static Type GetStaticType() {
		return Event::NETWORK_LAN_HOST_FOUND;
	}

	ULONG getIp() { return m_ip; }
	USHORT getHostPort() { return m_hostPort; }
	std::string getDesc() { return m_description; }

private:
	ULONG m_ip = 0;
	USHORT m_hostPort;
	std::string m_description = "";
};