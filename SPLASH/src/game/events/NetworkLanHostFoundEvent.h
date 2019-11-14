#pragma once
#include "Sail/events/Event.h"

struct NetworkLanHostFoundEvent : public Event {
	NetworkLanHostFoundEvent(const ULONG lobbyInfo, const USHORT _hostPort, const std::string& _desc)
		: Event(Event::Type::NETWORK_LAN_HOST_FOUND)
		, ip(lobbyInfo)
		, hostPort(_hostPort)
		, desc(_desc) { }
	~NetworkLanHostFoundEvent() = default;

	const ULONG ip = 0;
	const USHORT hostPort;
	const std::string desc = "";
};