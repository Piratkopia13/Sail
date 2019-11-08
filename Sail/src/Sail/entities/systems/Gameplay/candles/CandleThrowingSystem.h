#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class CandleThrowingSystem final : public BaseComponentSystem {
public:
	CandleThrowingSystem();
	~CandleThrowingSystem();

	void update(float dt) override;

private:
	void throwCandle(Entity* e);

};