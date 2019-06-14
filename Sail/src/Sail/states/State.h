#pragma once

#include <memory>
#include "StateIdentifiers.h"
#include "../events/IEventListener.h"

// Forward declare the StateStack
class StateStack;

// Base class for all states
class State : public IEventListener {

	public:
		typedef std::unique_ptr<State> Ptr;

	public:
		State(StateStack& stack);
		virtual ~State();

		// Process input for the state
		virtual bool processInput(float dt) = 0;
		// Updates the state
		virtual bool update(float dt) = 0;
		// Renders the state
		virtual bool render(float dt) = 0;
		// Renders imgui
		virtual bool renderImgui(float dt);
		// Sends events to the state
		virtual bool onEvent(Event& event) { return true; }

	protected:
		// Request the push of a new state to the stack next update
		void requestStackPush(States::ID stateID);
		// Request to pop the top state in the stack next update
		void requestStackPop();
		// Request to clear the stack in the next update
		void requestStackClear();

	private:
		// Pointer to the stack
		StateStack* m_stack;

};