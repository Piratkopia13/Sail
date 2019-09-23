#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class LightSystem final : public BaseComponentSystem {
public:
	LightSystem();
	~LightSystem();


	void update(float dt) override;
private:


};