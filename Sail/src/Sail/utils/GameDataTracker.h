#pragma once


struct statistics {
	LARGE_INTEGER bulletsFired;
	LARGE_INTEGER bulletsHit;
	LARGE_INTEGER bulletsHitPercentage;
	LARGE_INTEGER enemiesKilled;
	LARGE_INTEGER metresWalked;
	LARGE_INTEGER jumpsMade;
};

class GameDataTracker {
public:
	//
	~GameDataTracker();

	//
	void logWeaponFired();
	void logEnemyHit();
	void logEnemyKilled();
	void logJump();
	void logDistanceWalked(glm::vec3 vector);	// Does ABS(vector) internally

	// 
	void resetData();
	const statistics& getStatistics();

	//
	void renderImgui();

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