#pragma once
#include "Sail/events/Event.h"

class Entity;

struct IgniteCandleEvent : public Event {
	IgniteCandleEvent(Entity* _candle)
		: Event(Event::Type::IGNITE_CANDLE)
		, candle(_candle) {}

	Entity* candle;
};