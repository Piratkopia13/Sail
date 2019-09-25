#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/ai/pathfinding/NodeSystem.h"

class TransformComponent;
class PhysicsComponent;
class AiComponent;
class GunComponent;
class NodeSystem;
class Model;
class Octree;

#ifdef _DEBUG_NODESYSTEM
class Shader;
class Scene;
#endif

class AiSystem final : public BaseComponentSystem {
public:
	AiSystem();
	~AiSystem();

	void initNodeSystem(Model* bbModel, Octree* octree);
#ifdef _DEBUG_NODESYSTEM
	void initNodeSystem(Model* bbModel, Octree* octree, Shader* shader, Scene* scene);
#endif

	/*
		Adds an entity to the system
	*/
	bool addEntity(Entity* entity) override;

	std::vector<Entity*>& getEntities();

	void update(float dt) override;

private:
	struct AiEntity {
		TransformComponent* transComp;
		PhysicsComponent* physComp;
		AiComponent* aiComp;
	};

	void updatePath(AiComponent* aiComp, TransformComponent* transComp);
	void entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp);

private:
	std::unordered_map<int, AiEntity> m_aiEntities;

	float m_timeBetweenPathUpdate;

	std::unique_ptr<NodeSystem> m_nodeSystem;

	Octree* m_octree;

};