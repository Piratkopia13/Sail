#pragma once

#include "BoundingBox.h"
#include "Sail/entities/Entity.h"

class Model;
class Scene;

class Octree {
public:
	struct CollisionInfo {
		glm::vec3 normal;
		glm::vec3 positions[3];
		Entity* entity;
	};

private:
	struct Node {
		std::vector<Node> childNodes;
		Node* parentNode = nullptr;
		Entity::SPtr bbEntity;
		int nrOfEntities = 0;
		std::vector<Entity*> entities;
	};

	Node m_baseNode;

	Scene* m_scene;
	Model* m_boundingBoxModel;

	int m_softLimitMeshes;
	float m_minimumNodeHalfSize;

	void expandBaseNode(glm::vec3 direction);
	glm::vec3 findCornerOutside(Entity* entity, Node* testNode);
	bool addEntityRec(Entity* newEntity, Node* currentNode);
	bool removeEntityRec(Entity* entityToRemove, Node* currentNode);
	void updateRec(Node* currentNode, std::vector<Entity*>* entitiesToReAdd);
	void getCollisionData(Entity* entity, Entity* meshEntity, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, std::vector<Octree::CollisionInfo>* outCollisionData);
	void getCollisionsRec(Entity* entity, Node* currentNode, std::vector<Octree::CollisionInfo>* outCollisionData);
	int pruneTreeRec(Node* currentNode);

public:
	Octree(Scene *scene, Model *boundingBoxModel);
	~Octree();

	void addEntity(Entity* newEntity);
	void addEntities(std::vector<Entity*> *newEntities);

	void removeEntity(Entity* entityToRemove);
	void removeEntities(std::vector<Entity*> entitiesToRemove);

	void update();

	void getCollisions(Entity* entity, std::vector<CollisionInfo>* outCollisionData);
};