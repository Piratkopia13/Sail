#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class SoundComponent;

class HRTFAudioSystem final : public BaseComponentSystem {
public:
	HRTFAudioSystem();
	~HRTFAudioSystem();


	void update(float dt) override;

	void initializeSound(SoundComponent& sc);
	void updateSoundWithNewPosition(SoundComponent& sc);
	// TODO:
	//void update(float3 cameraPos, float3 fameraRot, etc)


};

