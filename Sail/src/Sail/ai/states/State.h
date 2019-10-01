#pragma once

#include <list>

typedef unsigned int FSMStateID;

FSMStateID global_fsmStateID = 0;

namespace FSM {
	struct Transition;

	class BaseState {
	public:
		BaseState() {}
		~BaseState() {}

		static FSMStateID createID() {
			return global_fsmStateID++;
		}

		void addTransition(Transition* transition, BaseState* toState) {
			m_transitions.emplace_back(transition, toState);
		}

		std::vector<std::pair<Transition*, BaseState*>>& getTransitions() {
			return m_transitions;
		}

		virtual void update(float dt) = 0;

		virtual void reset() = 0;

		virtual void init() = 0;

	protected:
		std::vector<std::pair<Transition*, BaseState*>> m_transitions;

	};

	template <typename StateType>
	class State : public BaseState {
	public:
		State() {}
		~State() {}

		virtual void update(float dt) = 0;

		virtual void reset() = 0;

		virtual void init() = 0;

		static const FSMStateID ID;

	};
	
	struct Transition {
		void addBoolCheck(bool* toCheck, bool value) {
			boolChecks.emplace_back(toCheck, value);
		}
		void addFloatLessThanCheck(float* toCheck, float value) {
			floatLessThanChecks.emplace_back(toCheck, value);
		}
		void addFloatEqualCheck(float* toCheck, float value) {
			floatEqualChecks.emplace_back(toCheck, value);
		}
		void addFloatGreaterThanCheck(float* toCheck, float value) {
			floatGreaterThanChecks.emplace_back(toCheck, value);
		}

		std::list<std::pair<bool*, bool>> boolChecks;
		std::list<std::pair<float*, float>> floatLessThanChecks;
		std::list<std::pair<float*, float>> floatEqualChecks;
		std::list<std::pair<float*, float>> floatGreaterThanChecks;
	};



	/*
		Defines the constant static ID of each state type at compile time
	*/
	template<typename StateType>
	const FSMStateID State<StateType>::ID = BaseState::createID();
}