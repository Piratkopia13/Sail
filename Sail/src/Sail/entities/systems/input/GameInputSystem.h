#pragma once
#include "..//BaseComponentSystem.h"

class GameInputSystem final : public BaseComponentSystem {
public:
	GameInputSystem();
	~GameInputSystem();

	void update(float dt) override;
	
	void processPerFrameInput();
	void processPerTickInput();

private:
};