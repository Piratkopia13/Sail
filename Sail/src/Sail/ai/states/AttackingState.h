#pragma once

#include "Sail/ai/states/State.h"
#include "Sail/utils/Utils.h"

class AiComponent;
class TransformComponent;
class GunComponent;
class PhysicsComponent;
class NodeSystem;
class Octree;

class AttackingState : public FSM::State<AttackingState> {
public:
	AttackingState(Octree* octree);
	~AttackingState();

	void update(float dt, Entity* entity) override;
	void reset(Entity* entity) override;
	void init(Entity* entity) override;

	void entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp);

private:
	Octree* m_octree;
};
