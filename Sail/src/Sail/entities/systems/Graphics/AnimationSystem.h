#pragma once
#include "..//BaseComponentSystem.h"
class Model;
class ModelComponent;

class AnimationSystem final : public BaseComponentSystem {
public:
	AnimationSystem();
	~AnimationSystem();

	void update(float dt) override;
	void updatePerFrame(float dt);

private:
	


};