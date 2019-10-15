#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class PlayerDeathEvent : public Event {
public:
	PlayerDeathEvent(int ID) : Event(Event::PLAYER_CANDLE_DEATH) {
		m_player_id = ID;
	}
	~PlayerDeathEvent() {}

	inline int getPlayerID() const { return m_player_id; };

	static Type GetStaticType() {
		return Event::PLAYER_CANDLE_DEATH;
	}

private:
	int m_player_id;

};
