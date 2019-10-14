#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class NetworkDisconnectEvent : public Event {
public:
	NetworkDisconnectEvent(int ID) : Event(Event::NETWORK_DISCONNECT) {
		m_player_id = ID;
	}
	~NetworkDisconnectEvent() {}

	inline int getPlayerID() const { return m_player_id; };

	static Type GetStaticType() {
		return Event::NETWORK_DISCONNECT;
	}

private:
	int m_player_id;

};