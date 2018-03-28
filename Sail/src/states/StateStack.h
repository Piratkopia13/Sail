#pragma once

#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cassert>

#include "State.h"
#include "StateIdentifiers.h"

// The stack containing different states of the game
// Allows for states to render on top of each other
class StateStack {

	public:
		// Actions that the stack can perform
		enum Action {
			Push,
			Pop,
			Clear
		};

	public:
		StateStack();

		// Registers a class to an id
		template <typename T>
		void registerState(States::ID stateID);

		// Passes input to the states in the stack
		virtual void processInput(float dt);
		// Process window resizing for the states in the stack
		virtual void resize(int width, int height);
		// Updates the states in the stack
		virtual void update(float dt);
		// Renders the states in the stack
		virtual void render(float dt);

		// Pushes a new state the next update
		void pushState(States::ID stateID);
		// Pops the top state the next update
		void popState();
		// Clears the stack the next update
		void clearStack();

		// Returns whether or not the stack is empty
		bool isEmpty() const;

	private:
		// Creates a new state instance from the id
		State::Ptr createState(States::ID stateID);
		// Applies all pending actions to the stack
		void applyPendingChanges();

	private:
		// Struct with information about a change
		struct PendingChange {
			PendingChange(Action action, States::ID stateID = States::None);

			Action action;
			States::ID stateID;
		};

	private:
		// The state stack itself
		std::vector<State::Ptr> m_stack;
		// A list of all changes that should be applied the next update
		std::vector<PendingChange> m_pendingList;
		// Maps functions to initialize a state by its id
		std::map<States::ID, std::function<State::Ptr()>> m_factories;


};

template <typename T>
void StateStack::registerState(States::ID stateID) {
	
	// Store a function to initialize a new pointer to the state
	m_factories[stateID] = [this]() {
		return State::Ptr(new T(*this));
	};

}