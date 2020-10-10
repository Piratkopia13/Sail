#pragma once

#include "Event.h"

class NewFrameEvent : public Event {
public:
	NewFrameEvent()
		: Event(Event::NEW_FRAME)
	{ }
	~NewFrameEvent() { };

	static Type getStaticType() {
		return Event::NEW_FRAME;
	}

};