#pragma once

#include "State.h"
#include "Sail/utils/Utils.h"

class AnotherTestState : public FSM::State<AnotherTestState> {
public:
	AnotherTestState() {

	}
	~AnotherTestState() {

	}

	void update(float dt) override {
		Logger::Log("I am AnotherTestState, hello!");
	}

	virtual void reset() {}

	virtual void init() {}



private:

};