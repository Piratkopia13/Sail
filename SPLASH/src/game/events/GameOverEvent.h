#pragma once
#include "Sail/events/Event.h"

struct GameOverEvent : public Event {
	GameOverEvent() : Event(Event::Type::GAME_OVER) {}
	~GameOverEvent() = default;
};