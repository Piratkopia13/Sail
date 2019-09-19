#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include <string>
using namespace std;

class NetworkNameEvent : public Event {
public:
	NetworkNameEvent(string name) 
		: Event(Event::NETWORK_NAME)
		, m_repliedName(name)
	{

	}
	~NetworkNameEvent() {}

	// When host calls it replies name, otherwise id.
	inline string getRepliedName() { return m_repliedName; }

	static Type getStaticType() {
		return Event::NETWORK_NAME;
	}

private:
	string m_repliedName;

};