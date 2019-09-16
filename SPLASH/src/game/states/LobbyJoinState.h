#pragma once

#include "LobbyState.h"
#include "Sail.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../SPLASH/src/game/events/NetworkWelcomeEvent.h"

class LobbyJoinState : public LobbyState {
public:
	LobbyJoinState(StateStack& stack);
	~LobbyJoinState();

	bool onEvent(Event& event);

private:
	bool onMyTextInput(TextInputEvent& event);
	bool onRecievedText(NetworkChatEvent& event);
	bool onPlayerJoined(NetworkJoinedEvent& event);
	bool onPlayerDisconnected(NetworkDisconnectEvent& event);
	bool onPlayerWelcomed(NetworkWelcomeEvent& event);
};