#pragma once

#include "Sail.h"
#include "Network/NWrapperSingleton.h"
#include "Network/NWrapperHost.h"
#include <string>
#include <list>
#include <ctime>

class NetworkLanHostFoundEvent;


class MenuState : public State, public EventHandler{
public:
	typedef std::unique_ptr<State> Ptr;

public:
	MenuState(StateStack& stack);
	virtual ~MenuState();

	// Process input for the state
	bool processInput(float dt);
	// Updates the state
	bool update(float dt, float alpha = 1.0f);
	// Renders the state
	bool render(float dt, float alpha = 1.0f);
	// Renders imgui
	bool renderImgui(float dt);
	// Sends events to the state
	bool onEvent(Event& event);

private:
	const std::string loadPlayerName(const std::string& file);
	bool loadModels(Application* app);
	bool loadTextures(Application* app);


private:
	Input* m_input = nullptr;
	// NetworkWrapper | NWrapperSingleton | NWrapperHost
	NWrapperSingleton* m_network = nullptr;
	Application* m_app = nullptr;
	// For ImGui Input
	std::string inputIP;
	std::future<bool> m_modelThread;

	// Other lobbies
	bool onLanHostFound(NetworkLanHostFoundEvent& event);
	//void sortFoundLobbies();
	void removeDeadLobbies();		// Only works with sorted lobbies
	const int m_ipBufferSize = 64;
	char* m_ipBuffer;
	int m_sleepTime;

	bool m_refreshing = true;
	 

	struct FoundLobby {
		std::string ip;
		std::string description;
		double duration = 20;
		void resetDuration() { duration = 20; }
	};
	double m_frameTick = 0.1;
	double udpChill = 5;
	double udpCounter = 0;
	std::vector<FoundLobby> m_foundLobbies;
	std::vector<std::string> m_newfoundLobbies;
};

