#pragma once
#include "..//BaseComponentSystem.h"

class UpdateBoundingBoxSystem final : public BaseComponentSystem
{
public:
	UpdateBoundingBoxSystem();
	~UpdateBoundingBoxSystem();

	void update(float dt) override;

private:
	void checkDistances(glm::vec3& minVec, glm::vec3& maxVec, const glm::vec3& testVec);
};