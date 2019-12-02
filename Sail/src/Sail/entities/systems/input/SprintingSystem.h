#pragma once


#include "../BaseComponentSystem.h"
#include "Sail/entities/Entity.h"

class SprintingSystem final : public BaseComponentSystem {
public:
	SprintingSystem();
	~SprintingSystem();

	void update(float dt, float alpha);
	void clean();
	void stop() override;

	void setCrosshair(Entity* pCrosshairEntity);

private:
	Entity* m_crosshair = nullptr;
};