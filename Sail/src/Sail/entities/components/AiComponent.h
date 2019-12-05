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
		, targetReachedThreshold(1.3f)
		, maxSteeringForce(0.3f)
		, mass(1.0f)
		, timeBetweenPathUpdate(3.f)
		, reachedPathingTarget(true)
		, updatePath(true)
		, doWalk(false)
		, automaticallyUpdatePath(true)
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
	float timeBetweenPathUpdate;
	
	bool reachedPathingTarget;
	bool updatePath;
	bool doWalk;
	bool automaticallyUpdatePath;

	int currNodeIndex;

	std::vector<NodeSystem::Node> currPath;
	NodeSystem::Node lastVisitedNode;

	Entity* controlledEntity;
	Entity* entityTarget;

	glm::vec3 posTarget;
	glm::vec3 lastTargetPos;

public:
#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this) + sizeof(NodeSystem::Node) * currPath.size();
	}

	void imguiRender(Entity** selected) {
		ImGui::Text(("movementSpeed " + std::to_string(movementSpeed)).c_str());
		ImGui::Text(("timeTakenOnPath " + std::to_string(timeTakenOnPath)).c_str());
		ImGui::Text(("targetReachedThreshold " + std::to_string(targetReachedThreshold)).c_str());
		ImGui::Text(("maxSteeringForce " + std::to_string(maxSteeringForce)).c_str());
		ImGui::Text(("mass " + std::to_string(mass)).c_str());
		ImGui::Text(("timeBetweenPathUpdate " + std::to_string(timeBetweenPathUpdate)).c_str());

		ImGui::Text(("reachedPathingTarget " + std::to_string(reachedPathingTarget)).c_str());
		ImGui::Text(("updatePath " + std::to_string(updatePath)).c_str());
		ImGui::Text(("doWalk " + std::to_string(doWalk)).c_str());
		ImGui::Text(("automaticallyUpdatePath " + std::to_string(automaticallyUpdatePath)).c_str());

		ImGui::Text(("currNodeIndex " + std::to_string(currNodeIndex)).c_str());

		ImGui::Text(("posTarget " + Utils::toStr(posTarget)).c_str());
		ImGui::Text(("lastTargetPos " + Utils::toStr(lastTargetPos)).c_str());
	}
#endif
};