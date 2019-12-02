#pragma once
#include "Sail/events/Event.h"

struct StopKillCamEvent : public Event {
	StopKillCamEvent() : Event(Event::Type::STOP_KILLCAM) {}
};