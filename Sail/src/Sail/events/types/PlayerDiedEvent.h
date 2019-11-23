#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;

struct PlayerDiedEvent : public Event {
	PlayerDiedEvent(Entity* _killed, Entity* _myPlayer, const Netcode::ComponentID _killerID, const Netcode::ComponentID _netIDkilled)
		: Event(Event::Type::PLAYER_DEATH)
		, killed(_killed)
		, myPlayer(_myPlayer)
		, killerID(_killerID)
		, netIDofKilled(_netIDkilled) {}

	Entity* killed;
	Entity* myPlayer;
	const Netcode::ComponentID killerID;
	const Netcode::ComponentID netIDofKilled;
};