#pragma once


#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkDroppedEvent : public Event {
public:
	NetworkDroppedEvent() : Event(Event::NETWORK_DROPPED)
	{

	}
	~NetworkDroppedEvent() {}

	static Type getStaticType() {
		return Event::NETWORK_DROPPED;
	}

private:
};