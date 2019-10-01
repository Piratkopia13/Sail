#pragma once

#include "State.h"
#include "Sail/utils/Utils.h"

class AttackingState : public FSM::State<AttackingState> {
public:
	AttackingState() {

	}
	~AttackingState() {

	}

	void update(float dt) override {
		Logger::Log("I am the Attacking State, hello!");
	}

	virtual void reset() {}

	virtual void init() {}



private:

};