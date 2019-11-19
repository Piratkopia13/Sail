#pragma once
#include "..//BaseComponentSystem.h"

class GameDataTracker;
class GunComponent;

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt) override;

#ifdef DEVELOPMENT
	unsigned int getByteSize() const override;
#endif
private:
	GameDataTracker* m_gameDataTracker = nullptr;

	float m_isLoopingStartingYet = 0.0f;
	bool m_shotStart = true;
	bool m_isLoopingShootSound = false;
	bool m_endSound = false;

	void alterProjectileSpeed(GunComponent* gun);

	void fireGun(Entity* e, GunComponent* gun);
	void overloadGun(Entity* e, GunComponent* gun);

	void setGunStateSTART(Entity* e, GunComponent* gun);
	void setGunStateLOOP(Entity* e, GunComponent* gun);
	void setGunStateEND(Entity* e, GunComponent* gun);

	void sendShootingEvents(Entity* e);
};