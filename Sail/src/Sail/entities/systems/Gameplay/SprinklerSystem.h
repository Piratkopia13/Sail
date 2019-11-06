#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"


class SprinklerSystem final : public BaseComponentSystem {
public:
	SprinklerSystem();
	~SprinklerSystem();

	void update(float dt) override;
	void stop() override;
	std::vector<int> getActiveRooms() const;

private:
	bool m_activateSprinklers = false;
	float m_endGameTimer = 0.f;
#ifdef _DEBUG
	float m_endGameStartLimit = 15.f;
#else
	float m_endGameStartLimit = 60.f;
#endif

	float m_endGameTimeIncrement = m_endGameStartLimit / 3.0f;
	float m_sprinklerDelayTime = 10.f;
	int m_endGameMapIncrement = 0;
	int m_xMinIncrement = 0;
	int m_xMaxIncrement = 0;
	int m_yMinIncrement = 0;
	int m_yMaxIncrement = 0;
	int m_mapSide = 0;
	std::vector<int> m_activeRooms;

	void addToActiveRooms(int room);

};