#pragma once

#include "Sail/ai/states/State.h"
#include "Sail/utils/Utils.h"

class AiComponent;
class TransformComponent;
class GunComponent;
class NodeSystem;
class Octree;

class AttackingState : public FSM::State<AttackingState> {
public:
	AttackingState();
	~AttackingState();

	void update(float dt, Entity* entity) override;
	void reset(Entity* entity) override;
	void init(Entity* entity) override;
	float* getDistToHost();

	void entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp, const glm::vec3& fireDir, const glm::vec3& gunPos, const float hitDist);

private:
	float m_distToHost;
};
