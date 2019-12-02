#include "LobbyClientState.h"

#include "Sail/events/EventDispatcher.h"
#include "Network/NWrapper.h"
#include "Network/NWrapperClient.h"
#include "Network/NWrapperSingleton.h"

LobbyClientState::LobbyClientState(StateStack& stack)
	: LobbyState(stack),
	m_wasDropped(false) {

	EventDispatcher::Instance().subscribe(Event::Type::NETWORK_DROPPED, this);
	EventDispatcher::Instance().subscribe(Event::Type::SETTINGS_UPDATED, this);
}

LobbyClientState::~LobbyClientState() {
	if (m_wasDropped) {
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();
	}

	EventDispatcher::Instance().unsubscribe(Event::Type::NETWORK_DROPPED, this);
	EventDispatcher::Instance().unsubscribe(Event::Type::SETTINGS_UPDATED, this);
}

bool LobbyClientState::onEvent(const Event& event) {
	LobbyState::onEvent(event);

	switch (event.type) {
	case Event::Type::NETWORK_DROPPED:		onDropped((const NetworkDroppedEvent&)event); break;
	case Event::Type::SETTINGS_UPDATED:		onSettingsChanged((const SettingsUpdatedEvent&)event); break;
	default: break;
	}
	return true;
}

bool LobbyClientState::onDropped(const NetworkDroppedEvent& event) {
	// Queue changes to statestack
	this->requestStackPop();
	this->requestStackPush(States::MainMenu);

	// Reset network so that user can choose host/client again.
	m_wasDropped = true;

	return true;
}

bool LobbyClientState::onSettingsChanged(const SettingsUpdatedEvent& event) {
	return true;
}
