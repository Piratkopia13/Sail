#pragma once
#include "Event.h"

class PlayerCandleHitEvent : public Event {
public:
	PlayerCandleHitEvent() : Event(Event::PLAYER_CANDLE_HIT) {
	}
	~PlayerCandleHitEvent() {
	}
	static Type getStaticType() {
		return Event::PLAYER_CANDLE_HIT;
	}
};