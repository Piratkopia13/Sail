#pragma once

#include "Sail/ai/states/State.h"

class NodeSystem;

class CleaningState : public FSM::State<CleaningState> {
public:
	CleaningState(NodeSystem* nodeSystem);
	~CleaningState(); 

	void update(float dt, Entity* entity) override;
	void reset(Entity* entity) override;
	void init(Entity* entity) override;

private:
	void findRandomNodeIndex(int currNodeIndex);

	NodeSystem* m_nodeSystem;
	int m_targetNode;
	float m_searchingClock;
};
