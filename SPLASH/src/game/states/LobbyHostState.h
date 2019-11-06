#pragma once

#include "LobbyState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../SPLASH/src/game/events/NetworkWelcomeEvent.h"

class LobbyHostState final : public LobbyState {
public:
	LobbyHostState(StateStack& stack);
	~LobbyHostState();

	bool onEvent(const Event& event) override;

private:
	bool onMyTextInput(const TextInputEvent& event);
	bool onRecievedText(const NetworkChatEvent& event);
	bool onPlayerJoined(const NetworkJoinedEvent& event);
	bool onPlayerDisconnected(const NetworkDisconnectEvent& event);
	bool onNameRequest(const NetworkNameEvent& event);

};