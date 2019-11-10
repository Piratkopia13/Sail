#pragma once

#include "LobbyState.h"
#include "Sail.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkChatEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "../SPLASH/src/game/events/NetworkDisconnectEvent.h"
#include "../SPLASH/src/game/events/NetworkWelcomeEvent.h"
#include "../SPLASH/src/game/events/NetworkNameEvent.h"
#include "../SPLASH/src/game/events/NetworkDroppedEvent.h"
#include "../SPLASH/src/game/events/NetworkStartGameEvent.h"
#include "../SPLASH/src/game/events/SettingsEvent.h"

class LobbyClientState final : public LobbyState {
public:
	LobbyClientState(StateStack& stack);
	~LobbyClientState();

	bool onEvent(const Event& event) override;

private:

	bool m_wasDropped;
	bool onMyTextInput(const TextInputEvent& event);
	bool onRecievedText(const NetworkChatEvent& event);
	bool onPlayerJoined(const NetworkJoinedEvent& event);
	bool onPlayerDisconnected(const NetworkDisconnectEvent& event);
	bool onDropped(const NetworkDroppedEvent& event);
	bool onStartGame(const NetworkStartGameEvent& event);
	bool onSettingsChanged(const SettingsUpdatedEvent& event);
};