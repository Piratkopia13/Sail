#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct PlayerLandedEvent : public Event {
	PlayerLandedEvent(const Netcode::ComponentID _netCompID)
		: Event(Event::Type::PLAYER_LANDED)
		, netCompID(_netCompID) {}
	const Netcode::ComponentID netCompID;
};
