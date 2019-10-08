#pragma once
#include "Event.h"

class PlayerCandleDeathEvent : public Event {
public:
	PlayerCandleDeathEvent() : Event(Event::PLAYER_CANDLE_DEATH) {
	}
	~PlayerCandleDeathEvent() {
	}
	static Type GetStaticType() {
		return Event::PLAYER_CANDLE_DEATH;
	}
};