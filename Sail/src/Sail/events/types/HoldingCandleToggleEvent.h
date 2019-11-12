#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct HoldingCandleToggleEvent : public Event {
	HoldingCandleToggleEvent(Netcode::CompID _netCompID, const bool _isHeld)
		: Event(Event::Type::HOLDING_CANDLE_TOGGLE)
		, netCompID(_netCompID)
		, isHeld(_isHeld) {}
	const Netcode::CompID netCompID;
	const bool isHeld;
};