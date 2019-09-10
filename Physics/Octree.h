#pragma once

#include "BoundingBox.h"

class Octree {
private:
	struct Node {
		std::vector<Node> childNodes;
		Node* parentNode;
		glm::vec3 nodeSize;
		glm::vec3 origin;
		BoundingBox bb;
		int nrOfMeshes;
		//std::vector<MeshBuilder*> meshes;
	};

	Node m_baseNode;

	int m_softLimitMeshes;
	float m_minimumNodeSize;

	void expandBaseNode(glm::vec3 direction);
	/*glm::vec3 findCornerOutside(MeshBuilder* mesh, Node* testNode);
	bool addMeshRec(MeshBuilder* newMesh, Node* currentNode);
	bool removeMeshRec(MeshBuilder* meshToRemove, Node* currentNode);
	void updateRec(Node* currentNode, std::vector<MeshBuilder*>* meshesToReAdd);
	void getCollisionsRec(BoundingBox* boundingBox, Node* currentNode, std::vector<CollisionInfo>* collisionData);
	float getRayIntersectionRec(glm::vec3 rayOrigin, glm::vec3 rayDirection, Node* currentNode);
	int drawRec(GLuint shaderProgram, Frustum* frustum, Node* currentNode);*/

public:
	Octree();
	~Octree();

	/*void addMesh(MeshBuilder* newMesh);
	void addMeshes(std::vector<MeshBuilder*> newMeshes);

	void removeMesh(MeshBuilder* meshToRemove);
	void removeMeshes(std::vector<MeshBuilder*> meshesToRemove);

	void update();

	void getCollisions(BoundingBox* boundingBox, std::vector<CollisionInfo>* collisionData);

	float getRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection);

	int draw(GLuint shaderProgram, Frustum* frustum);*/
};