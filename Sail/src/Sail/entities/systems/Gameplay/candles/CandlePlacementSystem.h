#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class Octree;

class CandlePlacementSystem final : public BaseComponentSystem {
public:
	CandlePlacementSystem();
	~CandlePlacementSystem();

	void update(float dt) override;

private:
	void toggleCandlePlacement(Entity* e);

};