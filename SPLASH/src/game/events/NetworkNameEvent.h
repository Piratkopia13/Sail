#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include <string>

class NetworkNameEvent : public Event {
public:
	NetworkNameEvent(std::string name)
		: Event(Event::NETWORK_NAME)
		, m_repliedName(name)
	{

	}
	~NetworkNameEvent() {}

	// When host calls it replies name, otherwise id.
	inline std::string getRepliedName() { return m_repliedName; }

	static Type GetStaticType() {
		return Event::NETWORK_NAME;
	}

private:
	std::string m_repliedName;

};