#pragma once

#include "Sail/events/Event.h"
#include "../states/LobbyState.h"
#include "Sail/netcode/NetcodeTypes.h"

struct NetworkDisconnectEvent : public Event {
	NetworkDisconnectEvent(const Netcode::PlayerID ID)
		: Event(Event::Type::NETWORK_DISCONNECT)
		, player_id(ID) { }
	~NetworkDisconnectEvent() = default;

	const Netcode::PlayerID player_id;
};