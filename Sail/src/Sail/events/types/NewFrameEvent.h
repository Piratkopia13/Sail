#pragma once
#include "../Event.h"

struct NewFrameEvent : public Event {
	NewFrameEvent() : Event(Event::Type::NEW_FRAME) {};
	~NewFrameEvent() = default;
};