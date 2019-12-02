#pragma once
#include "Sail/events/Event.h"
#include "Network/NWrapper.h"

struct NetworkLanHostFoundEvent : public Event {
	NetworkLanHostFoundEvent(const GameOnLanDescription& gameDescription)
		: Event(Event::Type::NETWORK_LAN_HOST_FOUND)
		, gameDescription(gameDescription) { }
	~NetworkLanHostFoundEvent() = default;

	const GameOnLanDescription gameDescription;
};