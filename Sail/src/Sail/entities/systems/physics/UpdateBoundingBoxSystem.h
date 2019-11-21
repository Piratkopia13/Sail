#pragma once
#include "..//BaseComponentSystem.h"

class UpdateBoundingBoxSystem final : public BaseComponentSystem
{
public:
	UpdateBoundingBoxSystem();
	~UpdateBoundingBoxSystem();

	bool addEntity(Entity* entity);

	void update(float dt) override;

private:
	void checkDistances(glm::vec3& minVec, glm::vec3& maxVec, const glm::vec3& testVec);
	void recalculateBoundingBoxFully(Entity* e);
	void recalculateBoundingBoxPosition(Entity* e);
	void updateRagdollBoundingBoxes(Entity* e);
};