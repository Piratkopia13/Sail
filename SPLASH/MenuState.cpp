#include "MenuState.h"


//#include "../imgui-sfml-master/imgui-SFML.h"
#include "..//libraries/imgui/imgui.h"
#include "..//Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"


MenuState::MenuState(StateStack& stack)
: State(stack)
{
	// Handle initialization of ImGui
	// --- Already initialized(?)
	//m_imGuiHandler = std::unique_ptr<ImGuiHandler>(ImGuiHandler::Create());
	//m_imGuiHandler->init();
}

MenuState::~MenuState()
{
	// Handle destruction of ImGui
	// ---
}

bool MenuState::processInput(float dt){
	// Did user want to enter anything in the chatbox?
	// Did user want to send the message?
	// ---



	// Did user want to change some setting?
	// ---

	// Did user want to start the game?
	// ---

	return false;
}

bool MenuState::update(float dt){
	// Did we send something?
	// ---

	// Did we recieve something?
	// Append the message in the proper order in the chatbox.
	// ---

	// Is anything going on in the background?
	// ---

	// Did we send something?
	// ---

	return false;
}

bool MenuState::render(float dt) {
	/// Maybe have m_ptr instead of fetching instance every frame.
	Application::getInstance()->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	


	return false;
}

bool MenuState::renderImgui(float dt) {
	/// Fetches width,height everyframe, maybe only fetch when window size changes?

	// Get coordinates of 0,0 --> width,height
	// so that the widgets etc can adapt to the size.
	unsigned int width = Application::getInstance()->getWindow()->getWindowWidth();
	unsigned int height = Application::getInstance()->getWindow()->getWindowHeight();
	unsigned int outerPadding = 15;
	unsigned int textHeight = 25;
	unsigned int titleHeight = 30;
	ImGuiWindowFlags generalFlags = ImGuiWindowFlags_NoCollapse;
	generalFlags |= ImGuiWindowFlags_NoResize;
	generalFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	generalFlags |= ImGuiWindowFlags_NoMove;

	// ------- PLAYER LIST ------- 
	ImGuiWindowFlags plFlags = generalFlags;
	plFlags |= ImGuiWindowFlags_NoTitleBar;
	const unsigned int nrOfPlayers = 3;
	std::string names[nrOfPlayers] = {
		"Shellow",
		"Series",
		"David"
	};
	ImGui::SetNextWindowPos(ImVec2(
		outerPadding,
		outerPadding
	));
	ImGui::Begin("Players in lobby:", NULL, plFlags);
	for (size_t i = 0; i < nrOfPlayers; i++) {
		ImGui::Text(
			std::string("- ").append(names[i].c_str()).c_str()
		);
	}
	ImGui::End();


	// ------- CHAT LOG ------- 
	const unsigned int nrOfMsg = 3;
	ImGuiWindowFlags chatFlags = generalFlags;

	std::string messages[nrOfMsg] = {
		"mesg1", "Message two", "Nr 3 mr.boss"
	};
	ImGui::SetNextWindowPos(ImVec2(
		outerPadding,
		height - (outerPadding + (2*height) / 10.0f)
	));
	ImGui::Begin("Chat Log", NULL, chatFlags);
	ImGui::SameLine();	// ???
	{
		ImGui::BeginChild(
			"Child1",
			ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5, 260)
		);

		for (size_t i = 0; i < nrOfMsg; i++)	{
			ImGui::Text(
				std::string("HH:MM:SS | ").append(
				names[i].append(" | ").c_str()).append(
				messages[i].c_str()).c_str()
			);
		}

		ImGui::EndChild();
	}



	ImGui::End();



	// ------- START BUTTON ------- 
	ImGui::SetNextWindowPos(ImVec2(
		width - (outerPadding + width / 10.0f),
		height - (outerPadding + height / 10.0f)
	));
	ImGui::Begin("Start The Game?  ", NULL, generalFlags);
	if (ImGui::Button("S.P.L.A.S.H")) {
		// Queue a removal of menustate, then a push of gamestate
		this->requestStackPop();
		this->requestStackPush(States::Game);
	}
	ImGui::End();





//	ImGui::SetNextWindowPos(ImVec2(100, 100));
	char* temp0 = nullptr;
//	ImGui::Text("asdfasdf");
	//	ImGui::InputText("asdf", temp0, 255);


	//Application::getInstance()->getImGuiHandler()->end();


	return false;
}
