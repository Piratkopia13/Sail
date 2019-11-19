#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;

struct TorchExtinguishedEvent : public Event {
	TorchExtinguishedEvent(const Netcode::PlayerID _shooterID, const Netcode::ComponentID _netIDextinguished)
		: Event(Event::Type::TORCH_EXTINGUISHED)
		, shooterID(_shooterID)
		, netIDextinguished(_netIDextinguished) {}

	const Netcode::PlayerID shooterID;
	const Netcode::ComponentID netIDextinguished;
};