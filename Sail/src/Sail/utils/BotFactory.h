#pragma once


#include "Sail.h"

class Model;

class BotFactory {
public:
	BotFactory();
	~BotFactory();

	void createBots(std::vector<glm::vec3> spawnPositions);

	void initialize();
	void reset();


private:
	//
	ECS* p_ECS = nullptr;

	//
	bool m_isInitialized = false;


	void createCharacterModel();


	// -+-+-+-+-+- Singleton requirements below -+-+-+-+-+-
public:
	BotFactory(BotFactory const&) = delete;
	void operator=(BotFactory const&) = delete;
	static BotFactory& getInstance() {
		static BotFactory instance;
		return instance;
	}
private:
	BotFactory();
};