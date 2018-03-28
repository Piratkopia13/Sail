#pragma once

#include "Sail.h"

class Game : public Application {

public:
	Game(HINSTANCE hInstance);
	~Game();

	virtual int run();
	virtual void processInput(float dt);
	virtual void update(float dt);
	virtual void render(float dt);
	virtual void resize(int width, int height);

private:
	// Register the different states
	void registerStates();

	StateStack m_stateStack;

};