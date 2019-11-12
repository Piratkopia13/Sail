#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct StopWalkingEvent : public Event {
	StopWalkingEvent(const Netcode::CompID _netCompID)
		: Event(Event::Type::STOP_WALKING)
		, netCompID(_netCompID) {}
	const Netcode::CompID netCompID;
};
