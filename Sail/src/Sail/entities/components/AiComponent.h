#pragma once

#include "Component.h"
#include "../Entity.h"
#include "Sail/ai/pathfinding/NodeSystem.h"
#include <glm/glm.hpp>

class AiComponent : public Component<AiComponent> {
public:
	AiComponent() 
		: movementSpeed(5.f)
		, timeTakenOnPath(0.f)
		, targetReachedThreshold(0.3f)
		, maxSteeringForce(0.3f)
		, mass(1.0f)
		, reachedTarget(true)
		, currNodeIndex(0)
		, lastVisitedNode(NodeSystem::Node(glm::vec3(10000.f, 10000.f, 10000.f), false, 2381831))
		, controlledEntity(nullptr)
		, entityTarget(nullptr)
		, lastTargetPos(glm::vec3(0.f, 0.f, 0.f))
		, posTarget(glm::vec3(0.f, 0.f, 0.f))
	{}

	void setTarget(Entity* entityTarget_);
	void setTarget(glm::vec3 targetPos);

	float movementSpeed;
	float timeTakenOnPath;
	float targetReachedThreshold;
	float maxSteeringForce;
	float mass;
	
	bool reachedTarget;

	int currNodeIndex;

	std::vector<NodeSystem::Node> currPath;
	NodeSystem::Node lastVisitedNode;

	Entity* controlledEntity;
	Entity* entityTarget;

	glm::vec3 lastTargetPos;
	glm::vec3 posTarget;
};