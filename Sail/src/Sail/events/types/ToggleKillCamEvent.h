#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;

struct ToggleKillCamEvent : public Event {
	ToggleKillCamEvent(const bool toggle, const Netcode::PlayerID killer = 0) 
		: Event(Event::Type::TOGGLE_KILLCAM), isActive(toggle), killedBy(killer)
	{}

	bool isActive;
	Netcode::PlayerID killedBy;
};