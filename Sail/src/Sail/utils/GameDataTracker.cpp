#include "pch.h"
#include "GameDataTracker.h"

GameDataTracker::GameDataTracker() {
	m_loggedData = { 0 };
}

GameDataTracker::~GameDataTracker() {

}

GameDataTracker& GameDataTracker::getInstance() {
	static GameDataTracker instance;

	return instance;
}

void GameDataTracker::resetData() {
	m_loggedData = { 0 };
}

void GameDataTracker::logWeaponFired() {
	m_loggedData.bulletsFired.QuadPart += 1;
}

void GameDataTracker::logEnemyHit() {
	m_loggedData.bulletsHit.QuadPart += 1;
}

void GameDataTracker::logEnemyKilled() {
	m_loggedData.enemiesKilled.QuadPart += 1;
}

void GameDataTracker::logJump() {
	m_loggedData.jumpsMade.QuadPart += 1;
}

void GameDataTracker::logDistanceWalked() {
}

const statistics& GameDataTracker::getStatistics() {
	return m_loggedData;
}

void GameDataTracker::renderImgui() {


}
