#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class NetworkJoinedEvent : public Event {
public:
	NetworkJoinedEvent(player player) : Event(Event::NETWORK_JOINED) {
		m_player = player;
	}
	~NetworkJoinedEvent() {}

	inline player getPlayer() const { return m_player; };

	static Type getStaticType() {
		return Event::NETWORK_JOINED;
	}

private:
	player m_player;

};