#pragma once

#include "Sail/events/Event.h"
#include "Sail/states/StateIdentifiers.h"

struct NetworkChangeStateEvent : public Event {
	NetworkChangeStateEvent(States::ID state, char stateData) : Event(Event::Type::NETWORK_CHANGE_STATE), stateID(state), stateData(stateData) { }
	~NetworkChangeStateEvent() = default;

	States::ID stateID;
	char stateData;
};