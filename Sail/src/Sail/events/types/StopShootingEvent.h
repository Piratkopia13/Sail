#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct StopShootingEvent : public Event {
	StopShootingEvent(const Netcode::CompID _netCompID)
		: Event(Event::Type::STOP_SHOOTING)
		, netCompID(_netCompID) {}
	const Netcode::CompID netCompID;
};