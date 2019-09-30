#pragma once

typedef double long LARGE_FLOAT;

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
	void logWeaponFired();						// ...GunSystem::update()
	void logEnemyHit();							// Nowhere atm
	void logEnemyKilled();						// Nowhere atm
	void logJump();								// ...GameInputSystem::update()
	void logDistanceWalked(glm::vec3 vector);	// ...PhysicsSystem::update()

	// 
	void resetData();
	const statistics& getStatistics();

	// Implemented in...
	void renderImgui();							// ...GameState::renderImGui()

private:
	statistics m_loggedData;
	

	// -+-+-+-+-+- Singleton requirements below -+-+-+-+-+-
public:
	GameDataTracker(GameDataTracker const&) = delete;
	void operator=(GameDataTracker const&) = delete;
	static GameDataTracker& getInstance();
private:
	GameDataTracker();
};