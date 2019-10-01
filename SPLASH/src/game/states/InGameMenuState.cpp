#include "InGameMenuState.h"

InGameMenuState::InGameMenuState(StateStack& stack) : State(stack) {

}

InGameMenuState::~InGameMenuState(){}

bool InGameMenuState::processInput(float dt) {
	return true;
}

bool InGameMenuState::update(float dt, float alpha) {
	return false;
}

bool InGameMenuState::fixedUpdate(float dt) {
	return false;
}

bool InGameMenuState::render(float dt, float alpha) {
	return false;
}