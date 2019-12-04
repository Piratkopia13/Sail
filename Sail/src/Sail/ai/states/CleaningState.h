#pragma once

#include "Sail/ai/states/State.h"

class NodeSystem;
class RendererWrapper;

class CleaningState : public FSM::State<CleaningState> {
public:
	CleaningState(NodeSystem* nodeSystem);
	~CleaningState(); 

	void update(float dt, Entity* entity) override;
	void reset(Entity* entity) override;
	void init(Entity* entity) override;

private:
	void findRandomNodeIndex(int currNodeIndex);
	void searchForWater(const glm::vec3& currPos);

	NodeSystem* m_nodeSystemRef;
	RendererWrapper* m_rendererWrapperRef;
	//int m_targetNode;
	glm::vec3 m_targetPos;
	float m_searchingClock;
};
