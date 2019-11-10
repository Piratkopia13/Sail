#pragma once

#include "Sail/events/Event.h"
#include "Network/NWrapper.h"

struct NetworkDisconnectEvent : public Event {
	NetworkDisconnectEvent(const Player _player)
		: Event(Event::Type::NETWORK_DISCONNECT)
		, player(_player) { }
	~NetworkDisconnectEvent() = default;

	const Player player;
};