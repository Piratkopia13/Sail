#pragma once

#include "State.h"
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
	void reset() override;
	void init() override;

	void entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp, Octree* octree);
	void updatePath(AiComponent* aiComp, TransformComponent* transComp, NodeSystem* nodeSystem);

private:

};