#pragma once

#include "../BaseComponentSystem.h"

class TransformComponent;
class PhysicsComponent;
class AiComponent;
class NodeSystem;
class Model;
class Octree;

class AiSystem final : public BaseComponentSystem {
public:
	AiSystem();
	~AiSystem();

	void initNodeSystem(Model* bbModel, Octree* octree);

	/*
		Adds an entity to the system
	*/
	virtual void addEntity(Entity* entity);

	/*
		Removes an entity from the system
	*/
	virtual void removeEntity(Entity* entity);

	std::vector<Entity*>& getEntities();

	void update(float dt) override;

private:
	struct AiEntity {
		TransformComponent* transComp;
		PhysicsComponent* physComp;
		AiComponent* aiComp;
	};

	void updatePath(AiComponent* aiComp, TransformComponent* transComp);

private:
	std::unordered_map<int, AiEntity> m_aiEntities;

	float m_timeBetweenPathUpdate;

	std::unique_ptr<NodeSystem> m_nodeSystem;

};