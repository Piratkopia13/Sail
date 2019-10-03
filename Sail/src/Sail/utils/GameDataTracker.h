#pragma once

typedef long double LARGE_FLOAT;

struct statistics {
	LARGE_INTEGER bulletsFired;
	LARGE_INTEGER bulletsHit;
	LARGE_INTEGER bulletsHitPercentage;
	LARGE_INTEGER enemiesKilled;
	LARGE_FLOAT distanceWalked;
	LARGE_INTEGER jumpsMade;
};

class GameDataTracker {
public:
	//
	~GameDataTracker();

	// Implemented in...
	void logWeaponFired();						// ...ehfy::update()
	void logEnemyHit();							// Nowhere atm
	void logEnemyKilled();						// Nowhere atm
	void logJump();								// ...GameInputSystem::update()
	void logDistanceWalked(glm::vec3 vector);	// ...PhysicsSystem::update()

	// Implemented in...
	void resetData();							// Nowhere atm
	const statistics& getStatistics();			// Nowhere atm

	// Implemented in...
	void renderImgui();							// ...EndState::renderImGui()

private:
	//
	statistics m_loggedData;
	

	// -+-+-+-+-+- Singleton requirements below -+-+-+-+-+-
public:
	GameDataTracker(GameDataTracker const&) = delete;
	void operator=(GameDataTracker const&) = delete;
	static GameDataTracker& getInstance();
private:
	GameDataTracker();
};