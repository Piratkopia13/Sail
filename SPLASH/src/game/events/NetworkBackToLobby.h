#pragma once

#include "Sail/events/Event.h"

struct NetworkBackToLobby : public Event {
	NetworkBackToLobby() : Event(Event::Type::NETWORK_BACK_TO_LOBBY) { }
	~NetworkBackToLobby() = default;
};
