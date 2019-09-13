#include "BobbyHostState.h"

#include "../SPLASH/src/game/events/TextInputEvent.h"

BobbyHostState::BobbyHostState(StateStack& stack)
	: BobbyState(stack)
{

}

BobbyHostState::~BobbyHostState() {
}

bool BobbyHostState::onEvent(Event& event)
{
	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onMyTextInput));
//	EventHandler::dispatch<RecievedMessageEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onRecievedText));
//	EventHandler::dispatch<PlayerJoinedEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onPlayerJoined));
//	EventHandler::dispatch<DisconnectEvent>(event, SAIL_BIND_EVENT(&BobbyHostState::onPlayerDisconnected));

	return false;
}

bool BobbyHostState::onMyTextInput(TextInputEvent& event)
{
	this->inputToChatLog(event.getMSG());

	return true;
}

bool BobbyHostState::onRecievedText() {

	return false;
}

bool BobbyHostState::onPlayerJoined() {
	std::string name = "name";
	unsigned int id = -1;
	this->BplayerJoined(name, id);

	return true;
}

bool BobbyHostState::onPlayerDisconnected() {
	unsigned int id = -1;
	this->BplayerLeft(id);
	
	return false;
}
