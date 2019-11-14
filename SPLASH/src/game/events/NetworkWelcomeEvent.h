#pragma once
#include <list>
#include "Sail/events/Event.h"
#include "../states/LobbyState.h"

struct NetworkWelcomeEvent : public Event {
	NetworkWelcomeEvent(const std::list<Player>& players) 
		: Event(Event::Type::NETWORK_WELCOME)
		, playerList(players) { }
	~NetworkWelcomeEvent() = default;

	const std::list<Player> playerList;
};