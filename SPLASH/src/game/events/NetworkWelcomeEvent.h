#pragma once


#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"
#include <list>

class NetworkWelcomeEvent : public Event {
public:
	NetworkWelcomeEvent(std::list<player> players) 
		: Event(Event::NETWORK_WELCOME) 
		, playerList(players)
	{

	}
	~NetworkWelcomeEvent() {}

	inline std::list<player> getListOfPlayers() { return playerList; }

	static Type getStaticType() {
		return Event::NETWORK_WELCOME;
	}

private:
	std::list<player> playerList;

};