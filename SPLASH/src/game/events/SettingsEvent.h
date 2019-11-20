#pragma once

#include "Sail/events/Event.h"
#include "../states/LobbyState.h"

struct SettingsUpdatedEvent : public Event {
	SettingsUpdatedEvent()
		: Event(Event::Type::SETTINGS_UPDATED) {
	}
	~SettingsUpdatedEvent() = default;
};