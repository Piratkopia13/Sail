#pragma once

#include "Sail/ai/states/State.h"
#include "Sail/utils/Utils.h"

class NodeSystem;

class FleeingState : public FSM::State<FleeingState> {
public:
	FleeingState(NodeSystem* nodeSystem);
	~FleeingState();

	void update(float dt, Entity* entity) override;
	void reset(Entity* entity) override;
	void init(Entity* entity) override;

private:
	int findBestNode(const glm::vec3& aiPos, Entity* enemy);

private:
	NodeSystem* m_nodeSystem;
};