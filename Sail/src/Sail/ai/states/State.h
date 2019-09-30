#pragma once

#include <list>

typedef unsigned int FSMStateID;

extern FSMStateID global_fsmStateID;

namespace FSM {
	class BaseState {
	public:
		BaseState() {}
		~BaseState() {}

		static FSMStateID createID() {
			return global_fsmStateID++;
		}

		/*
			Retrieves the number of states
		*/
		static int nrOfStateTypes() {
			return global_fsmStateID;
		}
	};

	class State : public BaseState {
	public:
		State() {}
		~State() {}

		static const FSMStateID ID;

		virtual void update() = 0;
	};

	class Transition {
	public:
		Transition() {}
		~Transition() {}

		void addBoolCheck(bool* toCheck, bool value) {
			m_boolChecks.emplace_back(toCheck, value);
		}
		void addFloatLessThanCheck(float* toCheck, float value) {
			m_floatLessThanChecks.emplace_back(toCheck, value);
		}
		void addFloatEqualCheck(float* toCheck, float value) {
			m_floatEqualChecks.emplace_back(toCheck, value);
		}
		void addFloatGreaterThanCheck(float* toCheck, float value) {
			m_floatGreaterThanChecks.emplace_back(toCheck, value);
		}

		bool checkTransition() {
			bool trans = true;
			for ( auto boolCheck : m_boolChecks ) {
				if ( *boolCheck.first != boolCheck.second ) {
					trans = false;
				}
			}

			for ( auto floatLessThanCheck : m_floatLessThanChecks ) {
				if ( *floatLessThanCheck.first < floatLessThanCheck.second ) {
					trans = false;
				}
			}

			for ( auto floatEqualCheck : m_floatEqualChecks ) {
				if ( std::abs(*floatEqualCheck.first - floatEqualCheck.second) < m_eps ) {
					trans = false;
				}
			}

			for ( auto floatGreaterThanCheck : m_floatGreaterThanChecks ) {
				if ( *floatGreaterThanCheck.first > floatGreaterThanCheck.second ) {
					trans = false;
				}
			}

			return trans;
		}

	private:
		// Epsilon
		float m_eps = 0.0001f;

		std::list<std::pair<bool*, bool>> m_boolChecks;
		std::list<std::pair<float*, float>> m_floatLessThanChecks;
		std::list<std::pair<float*, float>> m_floatEqualChecks;
		std::list<std::pair<float*, float>> m_floatGreaterThanChecks;
	};



	/*
		Defines the constant static ID of each component type at compile time
	*/
	const FSMStateID State::ID = BaseState::createID();
}