#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;

struct WaterHitPlayerEvent : public Event {
	WaterHitPlayerEvent(Entity* player, Entity* candle, Netcode::PlayerID _senderID)
		: Event(Event::Type::WATER_HIT_PLAYER)
		, hitPlayer(player)
		, hitCandle(candle)
		, senderID(_senderID) {}
	Entity* hitPlayer;
	Entity* hitCandle;
	const Netcode::PlayerID senderID;
};