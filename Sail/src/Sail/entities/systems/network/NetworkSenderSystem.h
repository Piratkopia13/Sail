#pragma once

#include "../BaseComponentSystem.h"

class NetworkSenderSystem : BaseComponentSystem {
public:
	NetworkSenderSystem();
	~NetworkSenderSystem();
	void update(float dt) override;

};