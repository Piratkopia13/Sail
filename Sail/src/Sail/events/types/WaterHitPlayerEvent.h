#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct WaterHitPlayerEvent : public Event {
	WaterHitPlayerEvent(Netcode::ComponentID _netCompID, Netcode::ComponentID _hitterID)
		: Event(Event::Type::WATER_HIT_PLAYER)
		, netCompID(_netCompID)
		, hitterID(_hitterID) {}
	const Netcode::ComponentID netCompID;
	const Netcode::ComponentID hitterID;
};