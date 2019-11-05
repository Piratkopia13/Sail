#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"


class SprinklerSystem final : public BaseComponentSystem {
public:
	SprinklerSystem();
	~SprinklerSystem();

	void update(float dt) override;

private:
	bool active = false;
	float endGameTimer = 0.f;
	float endGameStartLimit = 10.f;
	float endGameTimeIncrement = 5.f;
	float sprinklerDelayTime = 10.f;
	int endGameMapIncrement = 0;
	std::vector<int> activeRooms;

	void addToActiveRooms(int room);

};