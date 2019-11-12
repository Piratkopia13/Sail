#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct StartShootingEvent : public Event {
	StartShootingEvent(const Netcode::ComponentID _netCompID)
		: Event(Event::Type::START_SHOOTING)
		, netCompID(_netCompID) {}
	const Netcode::ComponentID netCompID;
};
