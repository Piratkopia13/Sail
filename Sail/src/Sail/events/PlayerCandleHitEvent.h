#pragma once
#include "Event.h"

class PlayerCandleHitEvent : public Event {
public:
	PlayerCandleHitEvent() : Event(Event::PLAYER_CANDLE_HIT) {
	}
	~PlayerCandleHitEvent() {
	}
	static Type GetStaticType() {
		return Event::PLAYER_CANDLE_HIT;
	}
};