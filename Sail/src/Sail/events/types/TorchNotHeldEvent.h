#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

struct TorchNotHeldEvent : public Event {
	TorchNotHeldEvent(Netcode::ComponentID _netCompID)
		: Event(Event::Type::TORCH_NOT_HELD)
		, netCompID(_netCompID) {
	}
	const Netcode::ComponentID netCompID;
};