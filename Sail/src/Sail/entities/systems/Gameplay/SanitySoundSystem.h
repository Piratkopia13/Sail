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

	bool switch_begin = false;
	bool switch_secondBeat = false;
	bool switch_ambiance = false;

	float heartBeatTimer = 0.0f;
	float heartSecondBeatThresh = 0.45f;
	float heartBeatResetThresh = 1.5f;
};