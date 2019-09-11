#pragma once

#include "BoundingBox.h"
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
		BoundingBox bb;
		int nrOfMeshes;
		std::vector<BoundingBox*> entities; //TODO: Replace this with entities that contain model and bounding box when component system is working properly
	};

	Node m_baseNode;

	Scene* m_scene;
	Model* m_boundingBoxModel;

	int m_softLimitMeshes;
	float m_minimumNodeHalfSize;

	void expandBaseNode(glm::vec3 direction);
	glm::vec3 findCornerOutside(BoundingBox* entity, Node* testNode);
	bool addEntityRec(BoundingBox* newEntity, Node* currentNode);
	bool removeEntityRec(BoundingBox* entityToRemove, Node* currentNode);
	void updateRec(Node* currentNode, std::vector<BoundingBox*>* entitiesToReAdd);
	void getCollisionsRec(BoundingBox* entity, Node* currentNode, std::vector<Octree::CollisionInfo>* collisionData);
	float getRayIntersectionRec(glm::vec3 rayOrigin, glm::vec3 rayDirection, Node* currentNode);
	/*int drawRec(GLuint shaderProgram, Frustum* frustum, Node* currentNode);*/

public:
	Octree(Scene *scene, Model *boundingBoxModel);
	~Octree();

	void addEntity(BoundingBox* newEntity);
	void addEntities(std::vector<BoundingBox*> *newEntities);

	void removeEntity(BoundingBox* entityToRemove);
	void removeEntities(std::vector<BoundingBox*> entitiesToRemove);

	void update();

	void getCollisions(BoundingBox* entity, std::vector<CollisionInfo>* collisionData);

	float getRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection);

	/*int draw(GLuint shaderProgram, Frustum* frustum);*/
};