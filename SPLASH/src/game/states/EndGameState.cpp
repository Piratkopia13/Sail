#include "EndGameState.h"
#include "Sail/entities/ECS.h"
#include "Sail/Application.h"
#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include "Sail/entities/systems/Systems.h"
#include "Sail/KeyBinds.h"
#include "Sail/utils/GameDataTracker.h"
#include "Network/NWrapperSingleton.h"
#include "../libraries/imgui/imgui.h"

EndGameState::EndGameState(StateStack& stack) : State(stack) {
	auto* ptr = Application::getInstance();
	auto* ecs = ECS::Instance();
	std::cout << "TEST";

	// Create network send and receive systems
	NWrapperSingleton::getInstance().setNSS(ECS::Instance()->getSystem<NetworkSenderSystem>());
	ECS::Instance()->getSystem<NetworkReceiverSystem>()->init(NWrapperSingleton::getInstance().getMyPlayerID(), this, ECS::Instance()->getSystem<NetworkSenderSystem>());
}

EndGameState::~EndGameState() {
	ECS::Instance()->stopAllSystems();
}

bool EndGameState::processInput(float dt) {
	return true;
}

bool EndGameState::update(float dt, float alpha) {
	NWrapperSingleton::getInstance().getNetworkWrapper()->checkForPackages();
	return true;
}
bool EndGameState::fixedUpdate(float dt) {


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

	if (NWrapperSingleton::getInstance().isHost()) {
		if (ImGui::Button("Lobby")) {
			//NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			//	Netcode::MessageType::SEND_ALL_BACK_TO_LOBBY,
			//	nullptr);

			Application::getInstance()->dispatchEvent(Event(Event::Type::NETWORK_BACK_TO_LOBBY)); 
			this->requestStackPop();
			this->requestStackPush(States::HostLobby);
		}
	}
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

bool EndGameState::onEvent(Event& event) {

	EventHandler::dispatch<NetworkBackToLobby>(event, SAIL_BIND_EVENT(&EndGameState::onReturnToLobby));

	return true;
}

bool EndGameState::onReturnToLobby(NetworkBackToLobby& event) {

	if (NWrapperSingleton::getInstance().isHost()) {
		std::string msg = "z";

		// Send it all clients
		NWrapperSingleton::getInstance().getNetworkWrapper()->sendMsgAllClients(msg);
	}
	else {
		std::cout << "MEOW";
		this->requestStackPop();
		this->requestStackPush(States::JoinLobby);
	}
	
	return true;
}

void EndGameState::updatePerTickComponentSystems(float dt)
{
}

void EndGameState::updatePerFrameComponentSystems(float dt, float alpha)
{
}

