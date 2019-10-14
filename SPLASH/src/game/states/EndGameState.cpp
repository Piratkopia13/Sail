#include "EndGameState.h"
#include "Sail/entities/ECS.h"
#include "Sail/Application.h"
#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include "Sail/KeyBinds.h"
#include "Sail/utils/GameDataTracker.h"
#include "Network/NWrapperSingleton.h"
#include "../libraries/imgui/imgui.h"

EndGameState::EndGameState(StateStack& stack) : State(stack) {

}

EndGameState::~EndGameState() {
}

bool EndGameState::processInput(float dt) {
	return true;
}

bool EndGameState::update(float dt, float alpha) {
	return true;
}

bool EndGameState::render(float dt, float alpha) {
	Application::getInstance()->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });

	// Call the empty draw function to just clear the screen until we have actual graphics.
	ECS::Instance()->getSystem<BeginEndFrameSystem>()->renderNothing();

	return true;
}

bool EndGameState::renderImgui(float dt) {
	ImGui::Begin("Game over");
	ImGui::SetWindowPos({500,500});
	ImGui::End();

	ImGui::Begin("Return");
	ImGui::SetWindowPos({ 500,550 });
	if (ImGui::Button("Main menu")) {
		NWrapperSingleton::getInstance().resetNetwork();
		NWrapperSingleton::getInstance().resetWrapper();
		this->requestStackPop();
		this->requestStackPush(States::MainMenu);
	}
	ImGui::End();

	ImGui::Begin("Quit");
	ImGui::SetWindowPos({ 500,600 });
	if (ImGui::Button("Quit Button")) {
		PostQuitMessage(0);
	}
	ImGui::End();

	GameDataTracker::getInstance().renderImgui();

	return true;
}
