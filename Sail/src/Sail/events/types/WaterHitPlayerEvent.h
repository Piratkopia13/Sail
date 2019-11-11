#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct WaterHitPlayerEvent : public Event {
	WaterHitPlayerEvent(Netcode::ComponentID _netCompID, Netcode::PlayerID _senderID)
		: Event(Event::Type::WATER_HIT_PLAYER)
		, netCompID(_netCompID)
		, senderID(_senderID) {}
	const Netcode::ComponentID netCompID;
	const Netcode::PlayerID senderID;
};