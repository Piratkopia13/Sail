#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"
#include "Sail/entities/Entity.h"

class CameraController;
class LightSetup;
class PerspectiveCamera;

class CandleSystem final : public BaseComponentSystem {
public:
	CandleSystem();
	virtual ~CandleSystem();

	void setPlayerCandle(Entity::SPtr candle);

	void lightCandle(std::string name);

	void update(float dt) override;

	void updatePlayerCandle(CameraController* cam, const float yaw);
private:
	Entity::SPtr m_playerCandle;

};