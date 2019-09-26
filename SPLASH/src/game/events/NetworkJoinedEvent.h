#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class NetworkJoinedEvent : public Event {
public:
	NetworkJoinedEvent(Player player) : Event(Event::NETWORK_JOINED) {
		m_player = player;
	}
	~NetworkJoinedEvent() {}

	inline Player getPlayer() const { return m_player; };

	static Type getStaticType() {
		return Event::NETWORK_JOINED;
	}

private:
	Player m_player;

};