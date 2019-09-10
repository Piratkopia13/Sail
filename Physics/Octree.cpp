#include "PhysicsPCH.h"

#include "Octree.h"

Octree::Octree() {
	m_softLimitMeshes = 6;
	m_minimumNodeSize = 15.0f;

	m_baseNode.origin = glm::vec3(-50.0f, -50.0f, -50.0f);
	m_baseNode.nodeSize = glm::vec3(100.0f, 100.0f, 100.0f);
	//m_baseNode.bb.createBoundingBox(m_baseNode.origin, m_baseNode.origin + m_baseNode.nodeSize);
	m_baseNode.parentNode = nullptr;
	m_baseNode.nrOfMeshes = 0;
}

Octree::~Octree() {

}

void Octree::expandBaseNode(glm::vec3 direction) {
	/*

	//Direction to expand in
	int x, y, z;
	x = direction.x <= 0.0f;
	y = direction.y <= 0.0f;
	z = direction.z <= 0.0f;

	Node newBaseNode;
	newBaseNode.origin = m_baseNode.origin - glm::vec3(x * m_baseNode.nodeSize.x, y * m_baseNode.nodeSize.y, z * m_baseNode.nodeSize.z);
	newBaseNode.nodeSize = m_baseNode.nodeSize * 2.0f;
	newBaseNode.bb.createBoundingBox(newBaseNode.origin, newBaseNode.origin + newBaseNode.nodeSize);
	newBaseNode.nrOfMeshes = 0;
	newBaseNode.parentNode = nullptr;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node tempChildNode;
				if (i == x && j == y && k == z) {
					tempChildNode = m_baseNode;
				}
				else {
					tempChildNode.nodeSize = m_baseNode.nodeSize;
					tempChildNode.origin = newBaseNode.origin + glm::vec3(tempChildNode.nodeSize.x * i, tempChildNode.nodeSize.y * j, tempChildNode.nodeSize.z * k);
					tempChildNode.bb.createBoundingBox(tempChildNode.origin, tempChildNode.origin + tempChildNode.nodeSize);
					tempChildNode.nrOfMeshes = 0;
				}
				tempChildNode.parentNode = &newBaseNode;
				newBaseNode.childNodes.push_back(tempChildNode);
			}
		}
	}

	m_baseNode = newBaseNode;
	*/
}

/*
glm::vec3 Octree::findCornerOutside(MeshBuilder* mesh, Node* testNode) {
	//Find if any corner of a mesh's bounding box is outside of node. Returns a vector towards the outside corner if one is found. Otherwise a 0.0f vec is returned.
	glm::vec3 directionVec(0.0f, 0.0f, 0.0f);

	std::vector<glm::vec3> corners = mesh->getBoundingBox()->getCorners();


	for (int i = 0; i < 8; i++) {
		glm::vec3 distanceVec = corners[i] - testNode->origin;

		if (distanceVec.x <= 0.0f || distanceVec.x >= testNode->nodeSize.x ||
			distanceVec.y <= 0.0f || distanceVec.y >= testNode->nodeSize.y ||
			distanceVec.z <= 0.0f || distanceVec.z >= testNode->nodeSize.z) {
			directionVec = distanceVec;
			i = 8;
		}
	}

	return directionVec;
}

bool Octree::addMeshRec(MeshBuilder* newMesh, Node* currentNode) {
	bool meshAdded = false;

	glm::vec3 isInsideVec = findCornerOutside(newMesh, currentNode);
	if (glm::length(isInsideVec) == 0.0f) {
		//The current node does contain the whole mesh. Keep going deeper or add to this node if no smaller nodes are allowed

		if (currentNode->childNodes.size() > 0) { //Not leaf node
			//Recursively call children

			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				meshAdded = addMeshRec(newMesh, &currentNode->childNodes[i]);

				if (meshAdded) {
					//Mesh was added to child node. Break loop. No need to try the rest of the children
					i = currentNode->childNodes.size();
				}
			}

			if (!meshAdded) { //Mesh did not fit in any child node
				//Add mesh to this node
				currentNode->meshes.push_back(newMesh);
				currentNode->nrOfMeshes++;
				meshAdded = true;
			}
		}
		else { //Is leaf node
			if (currentNode->nrOfMeshes < m_softLimitMeshes || currentNode->nodeSize.x / 2.0f < m_minimumNodeSize) { //Soft limit not reached or smaller nodes are not allowed
				//Add mesh to this node
				currentNode->meshes.push_back(newMesh);
				currentNode->nrOfMeshes++;
				meshAdded = true;
			}
			else {
				//Create more children
				for (int i = 0; i < 2; i++) {
					for (int j = 0; j < 2; j++) {
						for (int k = 0; k < 2; k++) {
							Node tempChildNode;
							tempChildNode.nodeSize = currentNode->nodeSize / 2.0f;
							tempChildNode.origin = currentNode->origin + glm::vec3(tempChildNode.nodeSize.x * i, tempChildNode.nodeSize.y * j, tempChildNode.nodeSize.z * k);
							tempChildNode.bb.createBoundingBox(tempChildNode.origin, tempChildNode.origin + tempChildNode.nodeSize);
							tempChildNode.nrOfMeshes = 0;
							tempChildNode.parentNode = currentNode;
							currentNode->childNodes.push_back(tempChildNode);

							//Try to put meshes that was in this leaf node in the new child nodes.
							for (int l = 0; l < currentNode->nrOfMeshes; l++) {
								if (addMeshRec(currentNode->meshes[l], &currentNode->childNodes.back())) {
									//Mesh was successfully added to child. Remove it from this node.
									currentNode->meshes.erase(currentNode->meshes.begin() + l);
									currentNode->nrOfMeshes--;
									l--;
								}
							}
						}
					}
				}

				//Try to add the mesh to newly created child nodes. It gets placed in current node within recursion if the children can not contain it.
				meshAdded = addMeshRec(newMesh, currentNode);
			}
		}
	}

	return meshAdded;
}

bool Octree::removeMeshRec(MeshBuilder* meshToRemove, Node* currentNode) {
	bool meshRemoved = false;

	//Look for mesh in this node
	for (int i = 0; i < currentNode->nrOfMeshes; i++) {
		if (currentNode->meshes[i] == meshToRemove) {
			//Mesh found - Remove it
			currentNode->meshes.erase(currentNode->meshes.begin() + i);
			currentNode->nrOfMeshes--;
			meshRemoved = true;
			i = currentNode->nrOfMeshes;
		}
	}

	if (!meshRemoved) {
		//Mesh was not in this node. Recursively call this function for the children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			if (removeMeshRec(meshToRemove, &currentNode->childNodes[i])) {
				//Mesh was removed by one of the children, break the loop
				meshRemoved = true;
				i = currentNode->childNodes.size();
			}
		}
	}

	return meshRemoved;
}

void Octree::updateRec(Node* currentNode, std::vector<MeshBuilder*>* meshesToReAdd) {
	for (int i = 0; i < currentNode->nrOfMeshes; i++) {
		if (currentNode->meshes[i]->getBoundingBox()->getChange()) { //Mesh has changed
			//Check if it is still in the right node
			if (glm::length(findCornerOutside(currentNode->meshes[i], currentNode)) > 0.0f) { //Is no longer contained in this node
				//Re-add the mesh to get it in the right node
				MeshBuilder* tempMesh = currentNode->meshes[i];
				//First remove the mesh from this node to avoid duplicates
				bool removed = removeMeshRec(currentNode->meshes[i], currentNode);

				if (removed) {
					i--;
					//Then store the mesh to re-add it to the tree in the right node
					meshesToReAdd->push_back(tempMesh);
				}
			}
		}
	}

	for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
		updateRec(&currentNode->childNodes[i], meshesToReAdd);
	}
}

void Octree::getCollisionsRec(BoundingBox* boundingBox, Node* currentNode, std::vector<CollisionInfo>* collisionData) {
	if (boundingBox->boundingBoxIntersection(&currentNode->bb)) { //Bounding box collides with the current node
		//Check against meshes
		for (int i = 0; i < currentNode->nrOfMeshes; i++) {
			if (boundingBox->boundingBoxIntersection(currentNode->meshes[i]->getBoundingBox())) { //Bounding box collides with mesh bounding box
				//Get collision
				for (int j = 0; j < currentNode->meshes[i]->getNumberOfVertices(); j += 3) {
					glm::vec3 tempCollision;
					tempCollision = boundingBox->triangleIntersection(currentNode->meshes[i]->getVertexPosition(j), currentNode->meshes[i]->getVertexPosition(j + 1), currentNode->meshes[i]->getVertexPosition(j + 2));

					if (glm::length(tempCollision) > 0.01f) {
						CollisionInfo tempInfo;
						tempInfo.normal = tempCollision;

						for (int k = 0; k < 3; k++) {
							tempInfo.positions[k] = currentNode->meshes[i]->getVertexPosition(j + k);
							tempInfo.uvs[k] = currentNode->meshes[i]->getVertexUvCoords(j + k);
						}

						tempInfo.mesh = currentNode->meshes[i];

						collisionData->push_back(tempInfo);
					}
				}
			}
		}

		//Check for children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			getCollisionsRec(boundingBox, &currentNode->childNodes[i], collisionData);
		}
	}
}

float Octree::getRayIntersectionRec(glm::vec3 rayOrigin, glm::vec3 rayDirection, Node* currentNode) {
	float returnValue = -1.0f;

	if (currentNode->bb.rayIntersection(rayOrigin, rayDirection)) { //Ray intersects this node
		//Check against meshes
		for (int i = 0; i < currentNode->nrOfMeshes; i++) {
			if (currentNode->meshes[i]->getBoundingBox()->rayIntersection(rayOrigin, rayDirection)) {
				//Test for intersection
				for (int j = 0; j < currentNode->meshes[i]->getNumberOfVertices(); j += 3) {
					float tempIntersection;
					glm::vec3 triangle[3] = { currentNode->meshes[i]->getVertexPosition(j), currentNode->meshes[i]->getVertexPosition(j + 1), currentNode->meshes[i]->getVertexPosition(j + 2) };
					tempIntersection = Intersections::rayTriTest(rayOrigin, rayDirection, triangle);

					//Save if closer than previous hit
					if (glm::length(tempIntersection) < returnValue || (returnValue < 0.0f && tempIntersection >= 0.0f)) {
						returnValue = tempIntersection;
					}
				}
			}
		}

		//Check for children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			float tempIntersection = getRayIntersectionRec(rayOrigin, rayDirection, &currentNode->childNodes[i]);

			//Save if closer than previous hit
			if (glm::length(tempIntersection) < returnValue || (returnValue < 0.0f && tempIntersection >= 0.0f)) {
				returnValue = tempIntersection;
			}
		}
	}

	return returnValue;
}

int Octree::drawRec(GLuint shaderProgram, Frustum* frustum, Node* currentNode) {
	int returnValue = 0;

	//Check if node is in frustum
	if (frustum->getCollision(&currentNode->bb)) {
		//In frustum

		//Draw meshes in node
		for (int i = 0; i < currentNode->nrOfMeshes; i++) {
			currentNode->meshes[i]->draw(shaderProgram);
			returnValue++;
		}

		//Call draw for all children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			returnValue += drawRec(shaderProgram, frustum, &currentNode->childNodes[i]);
		}
	}
	return returnValue;
}

void Octree::addMesh(MeshBuilder* newMesh) {
	//See if the base node needs to be bigger
	glm::vec3 directionVec = findCornerOutside(newMesh, &m_baseNode);

	if (glm::length(directionVec) != 0.0f) {
		//Position of vertex is outside base node
		//Create bigger base node
		expandBaseNode(directionVec);
		//Recall this function to try to add the mesh again
		addMesh(newMesh);
	}
	else {
		addMeshRec(newMesh, &m_baseNode);
	}
}

void Octree::addMeshes(std::vector<MeshBuilder*> newMeshes) {
	for (unsigned int i = 0; i < newMeshes.size(); i++) {
		addMesh(newMeshes[i]);
	}
}

void Octree::removeMesh(MeshBuilder* meshToRemove) {
	removeMeshRec(meshToRemove, &m_baseNode);
}

void Octree::removeMeshes(std::vector<MeshBuilder*> meshesToRemove) {
	for (unsigned int i = 0; i < meshesToRemove.size(); i++) {
		removeMesh(meshesToRemove[i]);
	}
}

void Octree::update() {
	std::vector<MeshBuilder*> meshesToReAdd;
	updateRec(&m_baseNode, &meshesToReAdd);

	for (unsigned int i = 0; i < meshesToReAdd.size(); i++) {
		addMesh(meshesToReAdd[i]);
	}

	meshesToReAdd.clear();
}

void Octree::getCollisions(BoundingBox* boundingBox, std::vector<CollisionInfo>* collisionData) {
	getCollisionsRec(boundingBox, &m_baseNode, collisionData);
}

float Octree::getRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection) {
	return getRayIntersectionRec(rayOrigin, rayDirection, &m_baseNode);
}

int Octree::draw(GLuint shaderProgram, Frustum* frustum) {
	return drawRec(shaderProgram, frustum, &m_baseNode);
}
*/