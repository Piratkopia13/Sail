#pragma once

#include "Sail/events/Event.h"
#include "../states/LobbyState.h"

struct SettingsUpdatedEvent : public Event {
	SettingsUpdatedEvent(const std::string& _settings)
		: Event(Event::Type::SETTINGS_UPDATED)
		, settings(_settings) {
	}
	~SettingsUpdatedEvent() = default;

	const std::string settings;
};