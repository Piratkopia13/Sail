#pragma once
#include <string>
#include "Sail/events/Event.h"

struct NetworkNameEvent : public Event {
	NetworkNameEvent(const std::string& name)
		: Event(Event::Type::NETWORK_NAME)
		, repliedName(name) { }
	~NetworkNameEvent() = default;

	const std::string repliedName;
};