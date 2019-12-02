#pragma once

#include "../../BaseComponentSystem.h"
#include "Sail/ai/pathfinding/NodeSystem.h"

class TransformComponent;
class AiComponent;
class GunComponent;
class MovementComponent;
class SpeedLimitComponent;
class NodeSystem;
class Model;
class Octree;
class MapComponent;

#ifdef _DEBUG_NODESYSTEM
class Shader;
#endif

class AiSystem final : public BaseComponentSystem {
public:
	AiSystem();
	~AiSystem();

	void initNodeSystem(Model* bbModel, Octree* octree);
#ifdef _DEBUG_NODESYSTEM
	void initNodeSystem(Model* bbModel, Octree* octree, Shader* shader);
#endif

	/*
		Adds an entity to the system
	*/
	//bool addEntity(Entity* entity) override;

	std::vector<Entity*>& getEntities();

	void update(float dt) override;

	NodeSystem* getNodeSystem();

#ifdef DEVELOPMENT
	unsigned int getByteSize() const override;
#endif

private:
	void updatePath(Entity* e);
	void updatePhysics(Entity* e, float dt);
	float getAiYaw(MovementComponent* moveComp, float currYaw, float dt);
	void aiUpdateFunc(Entity* e, const float dt);
	glm::vec3 getDesiredDir(AiComponent* aiComp, TransformComponent* transComp);
	bool nodeConnectionCheck(glm::vec3 nodePos, glm::vec3 otherNodePos);
	glm::vec3 getNodePos(const int x, const int z, float nodeSize, float nodePadding, float startOffsetX, float startOffsetZ);

private:
	float m_timeBetweenPathUpdate;

	std::unique_ptr<NodeSystem> m_nodeSystem;

	Octree* m_octree;

};