#include "EndGameState.h"
#include "Sail/entities/ECS.h"
#include "Sail/Application.h"
#include "Sail/entities/systems/render/RenderSystem.h"
#include "Sail/KeyBinds.h"


#include "../libraries/imgui/imgui.h"

EndGameState::EndGameState(StateStack& stack) : State(stack) {

}

EndGameState::~EndGameState() {
}

bool EndGameState::processInput(float dt) {
	if (Input::WasMouseButtonJustPressed(KeyBinds::disableCursor)) {
		Input::HideCursor(!Input::IsCursorHidden());
	}

	return true;
}

bool EndGameState::update(float dt, float alpha) {
	return true;
}

bool EndGameState::render(float dt, float alpha) {
	Application::getInstance()->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });

	// Call the empty draw function to just clear the screen until we have actual graphics.
	ECS::Instance()->getSystem<RenderSystem>()->draw();
	return true;
}

bool EndGameState::renderImgui(float dt) {

	ImGui::Begin("Game over");
	ImGui::End();

	ImGui::Begin("Return");
	if (ImGui::Button("Main menu")) {
		this->requestStackPop();
		this->requestStackPush(States::MainMenu);
	}
	ImGui::End();

	ImGui::Begin("Quit");
	if (ImGui::Button("Quit Button")) {
		PostQuitMessage(0);
	}
	ImGui::End();


	return true;
}
