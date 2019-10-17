#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class NetworkDisconnectEvent : public Event {
public:
	NetworkDisconnectEvent(unsigned char ID) : Event(Event::NETWORK_DISCONNECT) {
		m_player_id = ID;
	}
	~NetworkDisconnectEvent() {}

	inline unsigned char getPlayerID() const { return m_player_id; };

	static Type GetStaticType() {
		return Event::NETWORK_DISCONNECT;
	}

private:
	unsigned char m_player_id;

};