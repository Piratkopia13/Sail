#pragma once
#include "Sail/events/Event.h"

class GameOverEvent : public Event {
public:
	GameOverEvent()
		: Event(Event::GAME_OVER) {}
	~GameOverEvent() { }

	static Type GetStaticType() {
		return Event::GAME_OVER;
	}

};