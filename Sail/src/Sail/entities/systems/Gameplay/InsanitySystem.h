#pragma once
#include "..//BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"

class InsanitySystem final : public BaseComponentSystem, public EventReceiver {
public:
	InsanitySystem();
	~InsanitySystem();

	void update(float dt) override;	
	void updateInsanityNetworked(Netcode::ComponentID id, float insanity);

	float m_maxInsanity = 100;
	float m_minInsanity = 0;
private:
	bool onEvent(const Event& event) override;
};