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

	bool m_switch_heartBegin = false;
	bool m_switch_secondBeat = false;
	bool m_switch_ambiance = false;
	bool m_switch_breathing = false;

	float m_heartBeatTimer = 0.0f;
	float m_heartSecondBeatThresh = 0.45f;
	float m_heartBeatResetThresh = 1.5f;
};