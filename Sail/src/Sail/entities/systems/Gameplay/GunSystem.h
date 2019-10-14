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
};
