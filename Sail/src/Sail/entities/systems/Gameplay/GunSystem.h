#pragma once
#include "..//BaseComponentSystem.h"
#include "Sail/graphics/Scene.h"

class GameDataTracker;

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt, Scene* scene);
	void update(float dt) override;

private:
	GameDataTracker* m_gameDataTracker = nullptr;
};
