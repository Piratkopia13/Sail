#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct IgniteCandleEvent : public Event {
	IgniteCandleEvent(Netcode::CompID _netCompID)
		: Event(Event::Type::IGNITE_CANDLE)
		, netCompID(_netCompID)	{}

	const Netcode::CompID netCompID;
};