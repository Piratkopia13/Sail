#pragma once

#include "Sail/events/Event.h"

struct NetworkStartGameEvent : public Event {
	NetworkStartGameEvent() : Event(Event::Type::NETWORK_START_GAME) { }
	~NetworkStartGameEvent() = default;
};