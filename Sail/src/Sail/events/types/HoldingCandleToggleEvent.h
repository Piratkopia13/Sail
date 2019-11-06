#pragma once
#include "Sail/events/Event.h"
#include <glm/vec3.hpp>
class Entity;

struct HoldingCandleToggleEvent : public Event {
	HoldingCandleToggleEvent(Entity* _owner, Entity* _candle, const bool _isHeld, const glm::vec3& _pos)
		: Event(Event::Type::HOLDING_CANDLE_TOGGLE)
		, owner(_owner)
		, candle(_candle)
		, isHeld(_isHeld)
		, pos(_pos) {}
	Entity* owner;
	Entity* candle;
	const bool isHeld;
	glm::vec3 pos;
};