#pragma once

#include <chrono>

#include "Sail.h"

class Entity;
class TransformComponent;
class PhysicsComponent;

class AiController {
public:
	AiController();
	AiController(std::shared_ptr<Entity> toControl);
	~AiController();

	void update(float dt);

	void moveTo(glm::vec3 point);

	void chaseEntity(Entity* toChase);
	void setEntity(std::shared_ptr<Entity> toControl);

	std::shared_ptr<Entity> getEntity();
	Entity* getTargetEntity();

private:
	void updatePath();	

private:
	float m_movementSpeed;
	// In milliseconds
	float m_timeBetweenPathUpdate;
	float m_timeTaken;

	std::vector<NodeSystem::Node> m_currPath;
	int m_currNodeIndex;
	NodeSystem::Node m_lastVisitedNode;

	std::shared_ptr<Entity> m_controlledEntity;
	PhysicsComponent* m_physComp;
	TransformComponent* m_transComp;

	Entity* m_entityTarget;

	glm::vec3 m_lastTargetPos;
	glm::vec3 m_target;

	bool m_reachedTarget;
};