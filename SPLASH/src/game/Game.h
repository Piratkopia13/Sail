#pragma once

#include "Sail.h"
//#include "Physics.h"

class NetworkWrapper;

class Game : public Application {

public:
	Game(HINSTANCE hInstance);
	~Game();

	virtual int run() override;
	virtual void processInput(float dt) override;
	virtual void update(float dt) override;
	virtual void render(float dt, float alpha) override;
	virtual void dispatchEvent(Event& event) override;
	virtual void applyPendingStateChanges() override;

private:
	// Register the different states
	void registerStates();

	StateStack m_stateStack;
};