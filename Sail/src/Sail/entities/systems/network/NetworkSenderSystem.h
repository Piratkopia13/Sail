#pragma once

#include "../BaseComponentSystem.h"

class NetworkSenderSystem : public BaseComponentSystem {
public:
	NetworkSenderSystem();
	~NetworkSenderSystem();
	void update(float dt) override;
};