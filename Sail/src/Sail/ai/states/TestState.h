#pragma once

#include "State.h"
#include "Sail/utils/Utils.h"

class TestState : public FSM::State<TestState> {
public:
	TestState() {

	}
	~TestState() {

	}

	void update(float dt) override {
		Logger::Log("I am da test state, hello!");
	}

	virtual void reset() {}

	virtual void init() {}

	

private:
	
};