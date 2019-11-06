#pragma once
#include "Sail/events/Event.h"

struct ResetWaterEvent : public Event {
	ResetWaterEvent() : Event(Event::Type::RESET_WATER) {}
	~ResetWaterEvent() = default;
};