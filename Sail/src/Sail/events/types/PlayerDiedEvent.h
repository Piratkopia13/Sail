#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;

struct PlayerDiedEvent : public Event {
	PlayerDiedEvent(Entity* _killed, Entity* _myPlayer, const Netcode::PlayerID _shooterID, const Netcode::CompID _netIDkilled)
		: Event(Event::Type::PLAYER_DEATH)
		, killed(_killed)
		, myPlayer(_myPlayer)
		, shooterID(_shooterID)
		, netIDofKilled(_netIDkilled) {}

	Entity* killed;
	Entity* myPlayer;
	const Netcode::PlayerID shooterID;
	const Netcode::CompID netIDofKilled;
};