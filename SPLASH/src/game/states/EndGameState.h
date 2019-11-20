#pragma once

#include "Sail/states/State.h"
#include "Sail/utils/SailImGui/SailImGui.h"

struct NetworkJoinedEvent;
class Application;
class ImGuiHandler;

class EndGameState final : public State {
public:
	explicit EndGameState(StateStack& stack);
	~EndGameState();

	// Process input for the state
	bool processInput(float dt) override;
	// Updates the state
	bool update(float dt, float alpha = 1.0f) override;
	bool fixedUpdate(float dt) override;
	// Renders the state
	bool render(float dt, float alpha = 1.0f) override;
	// Renders imgui
	bool renderImgui(float dt) override;
	// Sends events to the state
	bool onEvent(const Event& event) override;

	bool onPlayerJoined(const NetworkJoinedEvent& event);

private:
	void updatePerTickComponentSystems(float dt);
	void updatePerFrameComponentSystems(float dt, float alpha);

	void renderMenu();
	void renderScore();
	void renderPersonalStats();
	void renderFunStats();

private:

	Application* m_app;
	ImGuiHandler* m_imguiHandler;

	ImGuiWindowFlags m_standaloneButtonflags;
	ImGuiWindowFlags m_backgroundOnlyflags;

	float m_padding;



};
