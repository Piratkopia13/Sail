#pragma once
#include "Sail/events/Event.h"
#include "Sail/netcode/NetcodeTypes.h"
#include "Sail/entities/systems/Audio/AudioData.h"

struct ChangeWalkingSoundEvent : public Event {
	ChangeWalkingSoundEvent(const Netcode::ComponentID _netCompID, const Audio::SoundType _soundType)
		: Event(Event::Type::CHANGE_WALKING_SOUND)
		, netCompID(_netCompID)
		, soundType(_soundType) {}
	const Netcode::ComponentID netCompID;
	const Audio::SoundType soundType;
};
