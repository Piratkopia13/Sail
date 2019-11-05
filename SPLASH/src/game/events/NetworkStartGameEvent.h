#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkStartGameEvent : public Event {
public:
	NetworkStartGameEvent(bool enterAsSpectator)
		: Event(Event::NETWORK_START_GAME) {
		m_enterAsSpectator = enterAsSpectator;
	}
	~NetworkStartGameEvent() {}


	static Type GetStaticType() {
		return Event::NETWORK_START_GAME;
	}

	bool getEnterAsSpectator() {
		return m_enterAsSpectator;
	}

private:
	bool m_enterAsSpectator = false;
};