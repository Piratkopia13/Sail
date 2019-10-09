#pragma once
#include "..//BaseComponentSystem.h"

class GameDataTracker;
class GunComponent;


namespace GunFactory {
	Entity* createWaterBullet(glm::vec3 pos, glm::vec3 dir, float projSpeed, int i);
}

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt);

	

private:
	GameDataTracker* m_gameDataTracker = nullptr;
};
