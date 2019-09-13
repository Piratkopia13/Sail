#include "BobbyState.h"


//#include "../imgui-sfml-master/imgui-SFML.h"
#include "../libraries/imgui/imgui.h"
#include "../Sail/src/API/DX12/imgui/DX12ImGuiHandler.h"
#include "../SPLASH/src/game/events/TextInputEvent.h"
#include "../SPLASH/src/game/events/NetworkJoinedEvent.h"
#include "Network/NetworkWrapper.h"

#include <string>
using namespace std;

BobbyState::BobbyState(StateStack& stack)
	: State(stack)
{
	// ImGui is already initiated and set up, thanks alex!
	m_app = Application::getInstance();
	m_input = Input::GetInstance();
	m_textHeight = 52;
	m_outerPadding = 15;

	m_BmessageLimit = 14;		// Should be based on size of chatbox instead of fixed
	m_BplayerLimit = 12;
	m_BplayerCount = 0;
	m_BmessageCount = 0;

	// Add local Bplayer as the first.
	m_myName = "Daniel";
	BplayerJoined(m_myName, m_tempID++);

	m_BmessageSizeLimit = 50;
	m_currentBmessageIndex = 0;
	m_currentBmessage = new char[m_BmessageSizeLimit] { 0 };

	// -------- test -------- 
	//addTestData();
}

BobbyState::~BobbyState() {
	delete[]m_currentBmessage;
}

bool BobbyState::processInput(float dt) {
	// Did user want to enter anything in the chatbox?
	// Did user want to send the Bmessage?
	// ---

	if (m_input->IsMouseButtonPressed(0)) {
		m_chatFocus = false;
	}

	// Purely a function for testing
//	this->doTestStuff();

	// Did user want to change some setting?
	// ---

	// Did user want to start the game?
	// ---

	return false;
}

bool BobbyState::onEvent(Event& event) {
	Logger::Log("Received event: " + std::to_string(event.getType()));

	EventHandler::dispatch<TextInputEvent>(event, SAIL_BIND_EVENT(&BobbyState::onTextInput));
	EventHandler::dispatch<NetworkJoinedEvent>(event, SAIL_BIND_EVENT(&BobbyState::onBplayerJoined));


	return false;
}

bool BobbyState::onTextInput(TextInputEvent& event)
{
	TextInputEvent* e = (TextInputEvent*) & event;
	MSG msg = e->getMSG();

	if (m_currentBmessageIndex < m_BmessageSizeLimit && msg.wParam != SAIL_KEY_RETURN) {
		// Add whichever button that was inputted to the current Bmessage
		// --- OBS : doesn't account for capslock, etc.
		m_currentBmessage[m_currentBmessageIndex++] = (char)msg.wParam;
	}
	if (msg.wParam == SAIL_KEY_RETURN && m_chatFocus == false) {
		sendBmessage(&string(m_currentBmessage));
	}

	return true;
}

void BobbyState::inputToChatLog(MSG& msg)
{
	if (m_currentBmessageIndex < m_BmessageSizeLimit && msg.wParam != SAIL_KEY_RETURN) {
		// Add whichever button that was inputted to the current Bmessage
		// --- OBS : doesn't account for capslock, etc.
		m_currentBmessage[m_currentBmessageIndex++] = (char)msg.wParam;
	}
	if (msg.wParam == SAIL_KEY_RETURN && m_chatFocus == false) {
		sendBmessage(&string(m_currentBmessage));
	}
}

bool BobbyState::update(float dt) {
	// Update screen dimensions & ImGui related
	// (Sure, events, but the only thing consuming resources is the BobbyState)
	this->m_screenWidth = m_app->getWindow()->getWindowWidth();
	this->m_screenHeight = m_app->getWindow()->getWindowHeight();

	// Did we send something?
	// ---
	if (NetworkWrapper::getInstance().isInitialized())
	{
		NetworkWrapper::getInstance().checkForPackages();
	}

	// Did we recieve something?
	// ---

	// Is anything going on in the background?
	// ---

	return false;
}

bool BobbyState::render(float dt) {
	m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });

	return false;
}

bool BobbyState::renderImgui(float dt) {
	// ------- Bplayer LIST ------- 
	renderBplayerList();

	// ------- CHAT LOG ------- 
	renderChat();

	// -------- SETTINGS ----------
	renderSettings();

	// ------- START BUTTON ------- 
	renderStartButton();

	return false;
}

bool BobbyState::BplayerJoined(string name, unsigned int id) {
	if (m_BplayerCount < m_BplayerLimit) {
		Bplayer newBplayer{
			id,
			name
		};
		m_Bplayers.push_back(newBplayer);
		m_BplayerCount++;
		return true;
	}
	return false;
}

bool BobbyState::BplayerLeft(unsigned int id) {
	// Linear search to get target 'Bplayer' struct, then erase that from the list
	Bplayer* toBeRemoved = nullptr;
	int pos = 0;
	for (auto BplayerIt : m_Bplayers) {
		if (BplayerIt.id == id) {
			toBeRemoved = &BplayerIt;
			m_Bplayers.remove(*toBeRemoved);
			return true;
		}
	}

	return false;
}

void BobbyState::sendBmessage(const string* text) {
	if (m_currentBmessageIndex != 0) { // If the Bmessage isn't empty
		this->addBmessageToChat(text, &m_Bplayers.front());

		// Reset currentBmessage
		m_currentBmessageIndex = 0;
		for (size_t i = 0; i < m_BmessageSizeLimit; i++) {
			m_currentBmessage[i] = '\0';
		}
	}
}

void BobbyState::recieveBmessage(string text, unsigned int senderID) {


	addBmessageToChat(&text, getBplayer(senderID));

}

void BobbyState::addBmessageToChat(const string* text, const Bplayer* sender) {
	// Add Bmessage to chatlog
	m_Bmessages.push_back(Bmessage{
		sender->name,
		*text
		});

	// New Bmessages replace old
	if (m_Bmessages.size() > m_BmessageLimit) {
		m_Bmessages.pop_front();
	}
}

Bplayer* BobbyState::getBplayer(unsigned int id) {
	for (auto BplayerIt : m_Bplayers) {
		if (BplayerIt.id == id) {
			return &BplayerIt;
		}
	}
	return nullptr;
}

void BobbyState::addTestData()
{
	// Set up Bplayers
	BplayerJoined("Ollie", m_tempID++);
	BplayerJoined("David", m_tempID++);
	//BplayerJoined("Press 0 to switch between enter msg and going to gamestate with mouse (MouseClick by default)");
	//BplayerJoined("The cause of this is an IMGUI-bug where keyboard focus prevents buttons from working");
	//BplayerJoined("This tape is since we'll switch from imgui later anyway, so why patch shit that's only an imgui bug");
	//BplayerJoined("If u want to look at it: https://github.com/ocornut/imgui/issues/455#event-1428367449");

	Bmessage msg;
	msg.sender = "Daniel";
	msg.content = "msg1";
	m_Bmessages.push_back(msg);

	msg.sender = "Ollie";
	msg.content = "Bmessage Two";
	m_Bmessages.push_back(msg);

	msg.sender = "David";
	msg.content = "msg 3 mr.boss";
	m_Bmessages.push_back(msg);
}

void BobbyState::doTestStuff()
{
	if (m_input->WasKeyJustPressed(SAIL_KEY_J)) {
		BplayerJoined("TestBplayer", m_tempID++);
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_U)) {
		BplayerLeft(1);
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_I)) {
		BplayerJoined("David", m_tempID++);
	}
	if (m_input->WasKeyJustPressed(SAIL_KEY_O)) {
		BplayerLeft(m_tempID--);
	}
}

void BobbyState::renderBplayerList() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_outerPadding
	));
	ImGui::Begin("Bplayers in lobby:", NULL, flags);

	for (auto currentBplayer : m_Bplayers) {
		ImGui::Text(
			std::string("- ").append(currentBplayer.name.c_str()).c_str()
		);
	}
	ImGui::End();
}

void BobbyState::renderStartButton() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowPos(ImVec2(
		m_screenWidth - (m_outerPadding + m_screenWidth / 10.0f),
		m_screenHeight - (m_outerPadding + m_screenHeight / 10.0f)
	));
	ImGui::Begin("Press 0 once", NULL, flags);

	// SetKeyBoardFocusHere on the chatbox prevents the button from working,
	// so if we click with the mouse, temporarily set focus to the button.

	if (ImGui::Button("S.P.L.A.S.H")) {
		// Queue a removal of BobbyState, then a push of gamestate
		this->requestStackPop();
		this->requestStackPush(States::Game);
	}
	ImGui::End();
}

void BobbyState::renderSettings() {

}

void BobbyState::renderChat() {
	ImGuiWindowFlags chatFlags = ImGuiWindowFlags_NoCollapse;
	chatFlags |= ImGuiWindowFlags_NoResize;
	chatFlags |= ImGuiWindowFlags_NoMove;
	chatFlags |= ImGuiWindowFlags_NoNav;
	chatFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	chatFlags |= ImGuiWindowFlags_NoTitleBar;
	chatFlags |= ImGuiWindowFlags_AlwaysAutoResize;

	// ------- Bmessage BOX ------- 
	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (m_outerPadding + m_textHeight)
	));
	ImGui::Begin(
		"Write Here",
		NULL,
		chatFlags
	);

	if (m_firstFrame) {
		ImGui::SetKeyboardFocusHere(-1);
		m_firstFrame = false;
		m_chatFocus = true;
	}
	ImGui::Text("Enter Bmessage:");
	if (ImGui::InputText("", m_currentBmessage, m_BmessageSizeLimit, ImGuiInputTextFlags_EnterReturnsTrue)) {
		this->sendBmessage(&(string)m_currentBmessage);
		ImGui::SetKeyboardFocusHere(-1);
		m_chatFocus = true;
	}
	ImGui::End();

	// ------- CHAT LOG ------- 
	ImGui::SetNextWindowSize(ImVec2(
		400,
		300
	));
	ImGui::SetNextWindowPos(ImVec2(
		m_outerPadding,
		m_screenHeight - (300 + m_outerPadding)
	));

	// Render Bmessage history
	ImGui::Begin("Chat Log", NULL, chatFlags);
	ImGui::SameLine();
	ImGui::BeginChild("Bmessages");
	for (auto currentBmessage : m_Bmessages) {
		std::string msg;
		msg.append("HH:MM:SS | ");
		msg.append(currentBmessage.sender);
		msg.append(" | ");
		msg.append(currentBmessage.content.c_str()).c_str();
		ImGui::Text(
			msg.c_str()
		);
	}
	ImGui::EndChild();
	ImGui::End();
}