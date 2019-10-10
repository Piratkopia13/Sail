#pragma once
#include "..//BaseComponentSystem.h"
#include "../../../Sail/src/Sail/netcode/NetworkedStructs.h"

class GameDataTracker;
class GunComponent;


namespace GunFactory {
	Entity* createWaterBullet(
		glm::vec3 pos, glm::vec3 dir,
		float projSpeed, int i,
		bool ownedLocally
	);
}

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt);

	

private:
	GameDataTracker* m_gameDataTracker = nullptr;
};
