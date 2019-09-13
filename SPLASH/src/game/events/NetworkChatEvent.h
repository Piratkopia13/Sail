#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkChatEvent : public Event{
public:
	NetworkChatEvent() : Event(Event::NETWORK_CHAT) {

	}
	~NetworkChatEvent() {}

	static Type getStaticType() {
		return Event::NETWORK_CHAT;
	}

private:

};