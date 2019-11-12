#pragma once
#include "..//BaseComponentSystem.h"
#include "Sail/netcode/NetworkedStructs.h"
#include "Sail/events/EventReceiver.h"

class SanitySoundSystem final : public BaseComponentSystem, public EventReceiver {
public:
	SanitySoundSystem();
	~SanitySoundSystem();

	void update(float dt) override;
private:
	bool onEvent(const Event& event) override;
};