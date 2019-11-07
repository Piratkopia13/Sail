#pragma once

#include "Sail/events/Event.h"

struct NetworkStartGameEvent : public Event {
	NetworkStartGameEvent(bool _isSpectator) : 
		Event(Event::Type::NETWORK_START_GAME), isSpectator(_isSpectator) { }
	~NetworkStartGameEvent() = default;
	const bool isSpectator;
};