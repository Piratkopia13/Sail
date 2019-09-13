#pragma once

#include "BobbyState.h"
#include "Sail.h"

class BobbyJoinState : public BobbyState {
public:
	BobbyJoinState(StateStack& stack);
	~BobbyJoinState();

	bool onEvent(Event& event);

private:
	bool onMyTextInput(TextInputEvent& event);
	bool onRecievedText(/*event*/);
	bool onPlayerJoined(/*event*/);
	bool onPlayerDisconnected(/*event*/);


};