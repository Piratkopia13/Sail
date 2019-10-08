#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class SoundComponent;
class TransformComponent;
class OmnidirectionalSound;
class Camera;

class HRTFAudioSystem final : public BaseComponentSystem {
public:
	HRTFAudioSystem();
	~HRTFAudioSystem();


	void update(float dt) override;
	void update(Camera& cam, float alpha) const;

	void initializeSound(OmnidirectionalSound& sc) const;
	void updateSoundWithNewPosition(OmnidirectionalSound& sound, Camera& cam, TransformComponent& transform, float alpha) const;
	// TODO:
	//void update(float3 cameraPos, float3 fameraRot, etc)


};

