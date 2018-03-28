#include "State.h"
#include "StateStack.h"

State::State(StateStack& stack) 
:  m_stack(&stack)
{

}

State::~State() {
}

void State::requestStackPush(States::ID stateID) {
	m_stack->pushState(stateID);
}
void State::requestStackPop() {
	m_stack->popState();
}
void State::requestStackClear() {
	m_stack->clearStack();
}
