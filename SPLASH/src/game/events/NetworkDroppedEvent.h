#pragma once
#include "Sail/events/Event.h"

struct NetworkDroppedEvent : public Event {
	NetworkDroppedEvent() : Event(Event::Type::NETWORK_DROPPED) { }
	~NetworkDroppedEvent() = default;
};