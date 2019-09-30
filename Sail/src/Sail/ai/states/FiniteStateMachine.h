#pragma once

#include <unordered_map>
#include <vector>

#include "State.h"

class FiniteStateMachine {
public:
	virtual void update(float dt) = 0;

	void addState(FSM::State* toAdd) {
		m_state.emplace(toAdd->ID, toAdd);
	}

	void addTransition(FSM::State* fromState, FSM::Transition* toAdd) {
		auto it = m_transitions.find(fromState->ID);
		if ( it != m_transitions.end() ) {
			it->second.emplace_back(toAdd);
		} else {
			std::vector<FSM::Transition*> transitions;
			transitions.emplace_back(toAdd);
			m_transitions.emplace(fromState->ID, transitions);
		}
	}

protected:
	std::unordered_map<FSMStateID, FSM::State*> m_state;
	std::unordered_map<FSMStateID, std::vector<FSM::Transition*>> m_transitions;

};