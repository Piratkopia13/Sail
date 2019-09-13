#include "BobbyJoinState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"

BobbyJoinState::BobbyJoinState(StateStack& stack)
	: BobbyState(stack)
{

}

BobbyJoinState::~BobbyJoinState() {
}

bool BobbyJoinState::onEvent(Event& event) {
	return false;

	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&BobbyJoinState::onMyTextInput));
	//	EventHandler::dispatch<RecievedMessageEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onRecievedText));
	//	EventHandler::dispatch<PlayerJoinedEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onPlayerJoined));
	//	EventHandler::dispatch<DisconnectEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onPlayerDisconnected));
}

bool BobbyJoinState::onMyTextInput(TextInputEvent& event) {
	this->inputToChatLog(event.getMSG());
	
	return false;
}

bool BobbyJoinState::onRecievedText() {

	return false;
}

bool BobbyJoinState::onPlayerJoined() {
	return false;
}

bool BobbyJoinState::onPlayerDisconnected() {
	return false;
}
