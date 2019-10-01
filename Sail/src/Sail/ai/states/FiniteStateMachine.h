#pragma once


#include <unordered_map>
#include <string>

#include "State.h"
#include "Sail/utils/Utils.h"

class FiniteStateMachine {
public:
	FiniteStateMachine()
		: m_currentState(nullptr) 
		, m_name("")
	{}

	FiniteStateMachine(const std::string& name)
		: FiniteStateMachine()
	{
		m_name = name;
	}

	virtual ~FiniteStateMachine() {
		for ( int i = 0; i < m_states.size(); i++ ) {
			Memory::SafeDelete(m_states[i]);
		}
	}

	virtual void update(float dt) {
		for ( auto transition : m_currentState->getTransitions() ) {
			if ( transition.first ) {
				m_currentState->reset();
				m_currentState = transition.second;
				m_currentState->init();
			}
		}

		m_currentState->update(dt);
	}

	/**
	 *	Creates the state type and adds it to the FSM, the first created state type will become the initial state of the FSM
	 *	
	 *	@tparam StateType the state type to be created
	 *	@tparam Args the arguments to send to the constructor of StateType
	 *	@param args the arguments
	 */
	template <typename StateType, typename... Args>
	void createState(Args... args);

	/**
	 *	Adds the inputed transition between FromStateType and ToStateType, this requires both FromStateType and ToStateType to already exist in the system
	 *	Deletion of toAdd is handled by the FSM
	 *
	 *  @tparam FromStateType, ToStateType the state types to transition from and to
	 *	@param toAdd the transition that defines when to transition
	 */
	template <typename FromStateType, typename ToStateType>
	bool addTransition(FSM::Transition* toAdd);

protected:
	bool checkTransition(FSM::Transition* transition) {
		bool trans = true;
		for ( auto boolCheck : transition->boolChecks ) {
			if ( *boolCheck.first != boolCheck.second ) {
				trans = false;
			}
		}

		for ( auto floatLessThanCheck : transition->floatLessThanChecks ) {
			if ( *floatLessThanCheck.first < floatLessThanCheck.second ) {
				trans = false;
			}
		}

		for ( auto floatEqualCheck : transition->floatEqualChecks ) {
			if ( std::abs(*floatEqualCheck.first - floatEqualCheck.second) < m_eps ) {
				trans = false;
			}
		}

		for ( auto floatGreaterThanCheck : transition->floatGreaterThanChecks ) {
			if ( *floatGreaterThanCheck.first > floatGreaterThanCheck.second ) {
				trans = false;
			}
		}

		return trans;
	}

protected:
	std::string m_name;
	FSM::BaseState* m_currentState;
	std::unordered_map<FSMStateID, FSM::BaseState*> m_states;

private:
	// Epsilon
	float m_eps = 0.0001f;
};

template<typename StateType, typename... Args>
inline void FiniteStateMachine::createState(Args... args) {
	auto it = m_states.find(StateType::ID);

	if ( it == m_states.end() ) {
		StateType* toEmplace = SAIL_NEW StateType(args...);
		m_states.emplace(StateType::ID, toEmplace);

		if ( m_currentState == nullptr ) {
			m_currentState = toEmplace;
		}
	} else {
		Logger::Error("Tried to create state type with ID " + std::to_string(StateType::ID) + " but it already existed in the FSM (" + m_name + ").");
	}
}

template<typename ToStateType, typename FromStateType>
inline bool FiniteStateMachine::addTransition(FSM::Transition* toAdd) {
	auto fromState = m_states.find(FromStateType::ID);
	if ( fromState != m_states.end() ) {
		auto toState = m_states.find(ToStateType::ID);
		if ( toState != m_states.end() ) {
			fromState->second->addTransition(toAdd, toState->second);
			return true;
		} else {
			Logger::Error("The to state state with ID: " + std::to_string(ToStateType::ID) + " was not found in the FSM (" + m_name + ").");
		}
	} else {
		Logger::Error("The from state state with ID: " + std::to_string(FromStateType::ID) + " was not found in the FSM (" + m_name + ").");
	}
	return false;
}
