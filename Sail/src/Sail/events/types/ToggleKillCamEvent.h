#pragma once
#include "Sail/events/Event.h"

class Entity;

struct ToggleKillCamEvent : public Event {
	ToggleKillCamEvent(const bool toggle) 
		: Event(Event::Type::TOGGLE_KILLCAM), isActive(toggle) 
	{}

	bool isActive;
};