#pragma once
#include "../Event.h"

struct PlayerCandleDeathEvent : public Event {
	PlayerCandleDeathEvent() : Event(Event::Type::PLAYER_CANDLE_DEATH) { }
	~PlayerCandleDeathEvent() = default;
};