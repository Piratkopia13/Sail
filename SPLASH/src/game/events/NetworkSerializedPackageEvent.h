#pragma once

#include <string>
#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkSerializedPackageEvent : public Event {
public:
	NetworkSerializedPackageEvent(std::string serializedData) 
		: Event(Event::NETWORK_SERIALIZED_DATA_RECIEVED)
		, m_serializedData(serializedData)
	{

	}
	~NetworkSerializedPackageEvent() { }

	std::string getSerializedData() { return m_serializedData; }

	static Type GetStaticType() {
		return Event::NETWORK_SERIALIZED_DATA_RECIEVED;
	}

private:
	std::string m_serializedData;
};



