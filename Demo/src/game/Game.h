#pragma once

#include "Sail.h"

class Game : public Application {

public:
	Game(HINSTANCE hInstance);
	~Game();

	virtual int run() override;
	virtual void processInput(float dt) override;
	virtual void update(float dt) override;
	virtual void render(float dt) override;

private:
	// Register the different states
	void registerStates();

	StateStack m_stateStack;

};