#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"


class SprinklerSystem final : public BaseComponentSystem {
public:
	SprinklerSystem();
	~SprinklerSystem();

	void update(float dt) override;
	std::vector<int> getActiveRooms() const;

private:
	bool activateSprinklers = false;
	float endGameTimer = 0.f;
	float endGameStartLimit = 10.f;
	float endGameTimeIncrement = 5.f;
	float sprinklerDelayTime = 10.f;
	int endGameMapIncrement = 0;
	int xMinIncrement = 0;
	int xMaxIncrement = 0;
	int yMinIncrement = 0;
	int yMaxIncrement = 0;
	std::vector<int> activeRooms;

	void addToActiveRooms(int room);

};