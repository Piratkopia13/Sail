#pragma once
#include "..//BaseComponentSystem.h"

class GameDataTracker;

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt) override;

private:
	GameDataTracker* m_gameDataTracker = nullptr;

	float m_isLoopingStartingYet = 0.0f;
	bool m_shotStart = true;
	bool m_isLoopingShootSound = false;
	bool m_endSound = false;
};