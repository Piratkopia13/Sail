#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct PlayerJumpedEvent : public Event {
	PlayerJumpedEvent(const Netcode::CompID _netCompID)
		: Event(Event::Type::PLAYER_JUMPED)
		, netCompID(_netCompID) {}
	const Netcode::CompID netCompID;
};
