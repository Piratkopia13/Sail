#pragma once

#include "BoundingBox.h"
#include "Sail/entities/ECS.h"
#include "Sail/graphics/Scene.h"
#include "Sail/graphics/geometry/Model.h"

class Octree {
public:
	struct CollisionInfo {
		glm::vec3 normal;
		glm::vec3 positions[3];
		glm::vec2 uvs[3];
		//Entity* entity;
	};

private:
	struct Node {
		std::vector<Node> childNodes;
		Node* parentNode;
		Entity::SPtr bbEntity;
		int nrOfEntities;
		std::vector<Entity::SPtr> entities; //TODO: Replace this with entities that contain model and bounding box when component system is working properly

		Node() {
			nrOfEntities = 0;
			parentNode = nullptr;
		}
	};

	Node m_baseNode;

	Scene* m_scene;
	Model* m_boundingBoxModel;

	int m_softLimitMeshes;
	float m_minimumNodeHalfSize;

	void expandBaseNode(glm::vec3 direction);
	glm::vec3 findCornerOutside(Entity::SPtr entity, Node* testNode);
	bool addEntityRec(Entity::SPtr newEntity, Node* currentNode);
	bool removeEntityRec(Entity::SPtr entityToRemove, Node* currentNode);
	void updateRec(Node* currentNode, std::vector<Entity::SPtr>* entitiesToReAdd);
	void getCollisionsRec(Entity::SPtr entity, Node* currentNode, std::vector<Octree::CollisionInfo>* collisionData);
	float getRayIntersectionRec(glm::vec3 rayOrigin, glm::vec3 rayDirection, Node* currentNode);
	int pruneTreeRec(Node* currentNode);

public:
	Octree(Scene *scene, Model *boundingBoxModel);
	~Octree();

	void addEntity(Entity::SPtr newEntity);
	void addEntities(std::vector<Entity::SPtr> *newEntities);

	void removeEntity(Entity::SPtr entityToRemove);
	void removeEntities(std::vector<Entity::SPtr> entitiesToRemove);

	void update();

	void getCollisions(Entity::SPtr entity, std::vector<CollisionInfo>* collisionData);

	float getRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection);
};