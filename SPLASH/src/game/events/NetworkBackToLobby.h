#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkBackToLobby : public Event {
public:
	NetworkBackToLobby()
	: Event(Event::NETWORK_BACK_TO_LOBBY)
	{ ; }
	~NetworkBackToLobby() { ; }

	static Type GetStaticType() {
		return Event::NETWORK_BACK_TO_LOBBY;
	}
};
