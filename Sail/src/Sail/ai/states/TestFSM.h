#pragma once

#include "FiniteStateMachine.h"

class TestFSM : public FiniteStateMachine {
public:
	TestFSM() {}
	~TestFSM() {}

	void update(float dt) override {
		Logger::Log("This is the TestFSM!");

		FiniteStateMachine::update(dt);
	}

private:

};