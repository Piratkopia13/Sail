#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkJoinedEvent : public Event {
public:
	NetworkJoinedEvent() : Event(Event::NETWORK_JOINED) {

	}
	~NetworkJoinedEvent() {}

	static Type getStaticType() {
		return Event::NETWORK_JOINED;
	}

private:


};