#pragma once

#include "BobbyState.h"

class BobbyHostState : public BobbyState {
public:
	BobbyHostState(StateStack& stack);
	~BobbyHostState();

	bool onEvent(Event& event);

private:
	bool onMyTextInput(TextInputEvent& event);
	bool onRecievedText(/*event*/);
	bool onPlayerJoined(/*event*/);
	bool onPlayerDisconnected(/*event*/);

};