#pragma once
#include "..//BaseComponentSystem.h"
class Model;
class ModelComponent;
class AnimationComponent;

class AnimationSystem final : public BaseComponentSystem {
public:
	AnimationSystem();
	~AnimationSystem();

	void update(float dt) override;
	void updatePerFrame(float dt);

private:
	
	void addTime(AnimationComponent* e, const float time);
	void interpolate(glm::mat4& res, const glm::mat4& mat1, const glm::mat4& mat2, const float w);

};