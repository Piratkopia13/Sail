#include "InGameMenuState.h"
#include "imgui.h"
#include "Sail/api/Input.h"
#include "Sail/Application.h"

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

bool InGameMenuState::renderImgui(float dt) {
	ImGui::Begin("Paused", NULL, ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar);
	ImVec2 size = ImVec2(50.f, 0.f);
	ImVec2 pos = ImVec2((Application::getInstance()->getWindow()->getWindowWidth() / 2.f) - (size.x / 2.f), (Application::getInstance()->getWindow()->getWindowHeight() / 2.f) - (size.y / 2.f));
	ImGui::SetWindowPos(pos);
	ImGui::SetWindowSize(size);
	ImGui::End();
	return false;
}