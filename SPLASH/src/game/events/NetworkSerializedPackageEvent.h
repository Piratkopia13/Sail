#pragma once
#include <string>
#include "Sail/events/Event.h"

struct NetworkSerializedPackageEvent : public Event {
	NetworkSerializedPackageEvent(const std::string& _serializedData)
		: Event(Event::Type::NETWORK_SERIALIZED_DATA_RECIEVED)
		, serializedData(_serializedData) { }
	~NetworkSerializedPackageEvent() = default;

	const std::string serializedData;
};
