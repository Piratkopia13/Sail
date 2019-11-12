#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct WaterHitPlayerEvent : public Event {
	WaterHitPlayerEvent(Netcode::CompID _netCompID, Netcode::PlayerID _senderID)
		: Event(Event::Type::WATER_HIT_PLAYER)
		, netCompID(_netCompID)
		, senderID(_senderID) {}
	const Netcode::CompID netCompID;
	const Netcode::PlayerID senderID;
};