#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkJoinedEvent : public Event {
public:
	NetworkJoinedEvent(int ID) : Event(Event::NETWORK_JOINED) {
		m_player_id = ID;
	}
	~NetworkJoinedEvent() {}

	inline int getPlayerID() const { return m_player_id; };

	static Type getStaticType() {
		return Event::NETWORK_JOINED;
	}

private:
	int m_player_id;

};