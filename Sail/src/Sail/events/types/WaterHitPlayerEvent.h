#pragma once
#include "../Event.h"

class Entity;

struct WaterHitPlayerEvent : public Event {
	WaterHitPlayerEvent(Entity* player, Entity* candle, unsigned char _senderID)
		: Event(Event::Type::WATER_HIT_PLAYER)
		, hitPlayer(player)
		, hitPlayer(candle)
		, senderID(_senderID) {}
	Entity* hitPlayer;
	Entity* hitCandle;
	const unsigned char senderID;
};