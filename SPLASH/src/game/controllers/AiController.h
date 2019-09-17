#pragma once

class Entity;
class GameTransformComponent;
class PhysicsComponent;

class AiController {
public:
	AiController();
	AiController(std::shared_ptr<Entity> toControl);
	~AiController();

	void update();

	void moveTo(glm::vec3 point);

	void chaseEntity(Entity* toChase);
	void setEntity(std::shared_ptr<Entity> toControl);

	std::shared_ptr<Entity> getEntity();
	Entity* getTargetEntity();
	

private:
	float m_movementSpeed = 5.f;

	std::shared_ptr<Entity> m_controlledEntity;
	PhysicsComponent* m_physComp;
	GameTransformComponent* m_transComp;

	Entity* m_entityTarget;

	glm::vec3 m_target;

	bool m_reachedTarget;
};