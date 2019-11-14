#pragma once

#include "Sail/events/Event.h"
#include "Network/NWrapper.h"

struct NetworkJoinedEvent : public Event {
	NetworkJoinedEvent(const Player _player) 
		: Event(Event::Type::NETWORK_JOINED)
		, player(_player) { }
	~NetworkJoinedEvent() = default;

	const Player player;
};