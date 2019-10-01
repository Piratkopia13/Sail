#pragma once

#include "FiniteStateMachine.h"

class TestFSM : public FiniteStateMachine {
public:
	TestFSM()
		: FiniteStateMachine() 
	{}

	TestFSM(const std::string& name)
		: FiniteStateMachine(name)
	{}

	~TestFSM() {}

	void update(float dt) override {
		Logger::Log("This is the " + m_name + "!");

		FiniteStateMachine::update(dt);
	}

private:

};