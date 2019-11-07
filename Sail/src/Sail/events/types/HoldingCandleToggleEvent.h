#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"
#include <glm/vec3.hpp>
class Entity;

struct HoldingCandleToggleEvent : public Event {
	HoldingCandleToggleEvent(Netcode::ComponentID _netCompID, const bool _isHeld, const glm::vec3& _pos)
		: Event(Event::Type::HOLDING_CANDLE_TOGGLE)
		, netCompID(_netCompID)
		, isHeld(_isHeld)
		, pos(_pos) {}
	const Netcode::ComponentID netCompID;
	const bool isHeld;
	const glm::vec3 pos;
};