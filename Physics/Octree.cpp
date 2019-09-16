#include "PhysicsPCH.h"

#include "Sail/entities/components/Components.h"

#include "Octree.h"

Octree::Octree(Scene* scene, Model* boundingBoxModel) {
	m_scene = scene;
	m_boundingBoxModel = boundingBoxModel;
	m_softLimitMeshes = 1;
	m_minimumNodeHalfSize = 5.0f;

	m_baseNode.bbEntity = ECS::Instance()->createEntity("Bounding Box");
	m_baseNode.bbEntity->addComponent<BoundingBoxComponent>(m_boundingBoxModel);
	BoundingBox* tempBoundingBox = m_baseNode.bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
	tempBoundingBox->setPosition(glm::vec3(0.0f));
	tempBoundingBox->setHalfSize(glm::vec3(50.0f, 50.0f, 50.0f));
	m_baseNode.bbEntity->addComponent<TransformComponent>(tempBoundingBox->getPosition(), glm::vec3(0.0f), tempBoundingBox->getHalfSize() * 2.0f);
	m_scene->addEntity(m_baseNode.bbEntity);
	m_baseNode.parentNode = nullptr;
	m_baseNode.nrOfEntities = 0;
}

Octree::~Octree() {

}

void Octree::expandBaseNode(glm::vec3 direction) {
	//Direction to expand in
	int x, y, z;
	x = direction.x >= 0.0f;
	y = direction.y >= 0.0f;
	z = direction.z >= 0.0f;

	Node newBaseNode;
	const BoundingBox* baseNodeBB = m_baseNode.bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
	newBaseNode.bbEntity = ECS::Instance()->createEntity("Bounding Box");
	newBaseNode.bbEntity->addComponent<BoundingBoxComponent>(m_boundingBoxModel);
	BoundingBox* newBaseNodeBoundingBox = newBaseNode.bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
	newBaseNodeBoundingBox->setPosition(baseNodeBB->getPosition() - baseNodeBB->getHalfSize() + glm::vec3(x * baseNodeBB->getHalfSize().x * 2.0f, y * baseNodeBB->getHalfSize().y * 2.0f, z * baseNodeBB->getHalfSize().z * 2.0f));
	newBaseNodeBoundingBox->setHalfSize(baseNodeBB->getHalfSize() * 2.0f);
	newBaseNode.bbEntity->addComponent<TransformComponent>(newBaseNodeBoundingBox->getPosition(), glm::vec3(0.0f), newBaseNodeBoundingBox->getHalfSize() * 2.0f);
	m_scene->addEntity(newBaseNode.bbEntity);
	newBaseNode.nrOfEntities = 0;
	newBaseNode.parentNode = nullptr;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node tempChildNode;
				if (i != x && j != y && k != z) {
					tempChildNode = m_baseNode;
				}
				else {
					tempChildNode.bbEntity = ECS::Instance()->createEntity("Bounding Box");
					tempChildNode.bbEntity->addComponent<BoundingBoxComponent>(m_boundingBoxModel);
					BoundingBox* tempChildBoundingBox = tempChildNode.bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
					tempChildBoundingBox->setHalfSize(baseNodeBB->getHalfSize());
					tempChildBoundingBox->setPosition(newBaseNodeBoundingBox->getPosition() - baseNodeBB->getHalfSize() + glm::vec3(tempChildBoundingBox->getHalfSize().x * 2.0f * i, tempChildBoundingBox->getHalfSize().y * 2.0f * j, tempChildBoundingBox->getHalfSize().z * 2.0f * k));
					tempChildNode.bbEntity->addComponent<TransformComponent>(tempChildBoundingBox->getPosition(), glm::vec3(0.0f), tempChildBoundingBox->getHalfSize() * 2.0f);
					m_scene->addEntity(tempChildNode.bbEntity);
					tempChildNode.nrOfEntities = 0;
				}
				tempChildNode.parentNode = &newBaseNode;
				newBaseNode.childNodes.push_back(tempChildNode);
			}
		}
	}

	m_baseNode = newBaseNode;
}


glm::vec3 Octree::findCornerOutside(Entity::SPtr entity, Node* testNode) {
	//Find if any corner of a entity's bounding box is outside of node. Returns a vector towards the outside corner if one is found. Otherwise a 0.0f vec is returned.
	glm::vec3 directionVec(0.0f, 0.0f, 0.0f);

	const std::vector<glm::vec3>* corners = entity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getCorners();
	glm::vec3 testNodeHalfSize = testNode->bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getHalfSize();

	for (int i = 0; i < 8; i++) {
		glm::vec3 distanceVec = corners->at(i) - testNode->bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getPosition();

		if (distanceVec.x <= -testNodeHalfSize.x || distanceVec.x >= testNodeHalfSize.x ||
			distanceVec.y <= -testNodeHalfSize.y || distanceVec.y >= testNodeHalfSize.y ||
			distanceVec.z <= -testNodeHalfSize.z || distanceVec.z >= testNodeHalfSize.z) {
			directionVec = distanceVec;
			i = 8;
		}
	}

	return directionVec;
}

bool Octree::addEntityRec(Entity::SPtr newEntity, Node* currentNode) {
	bool entityAdded = false;

	glm::vec3 isInsideVec = findCornerOutside(newEntity, currentNode);
	if (glm::length(isInsideVec) < 1.0f) {
		//The current node does contain the whole mesh. Keep going deeper or add to this node if no smaller nodes are allowed

		if (currentNode->childNodes.size() > 0) { //Not leaf node
			//Recursively call children

			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				entityAdded = addEntityRec(newEntity, &currentNode->childNodes[i]);

				if (entityAdded) {
					//Mesh was added to child node. Break loop. No need to try the rest of the children
					i = (unsigned int) currentNode->childNodes.size();
				}
			}

			if (!entityAdded) { //Mesh did not fit in any child node
				//Add mesh to this node
				currentNode->entities.push_back(newEntity);
				currentNode->nrOfEntities++;
				entityAdded = true;
			}
		}
		else { //Is leaf node
			if (currentNode->nrOfEntities < m_softLimitMeshes || currentNode->bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getHalfSize().x / 2.0f < m_minimumNodeHalfSize) { //Soft limit not reached or smaller nodes are not allowed
				//Add mesh to this node
				currentNode->entities.push_back(newEntity);
				currentNode->nrOfEntities++;
				entityAdded = true;
			}
			else {
				//Create more children
				for (int i = 0; i < 2; i++) {
					for (int j = 0; j < 2; j++) {
						for (int k = 0; k < 2; k++) {
							const BoundingBox* currentNodeBB = currentNode->bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
							Node tempChildNode;
							tempChildNode.bbEntity = ECS::Instance()->createEntity("Bounding Box");
							tempChildNode.bbEntity->addComponent<BoundingBoxComponent>(m_boundingBoxModel);
							BoundingBox* tempChildBoundingBox = tempChildNode.bbEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
							tempChildBoundingBox->setHalfSize(currentNodeBB->getHalfSize() / 2.0f);
							tempChildBoundingBox->setPosition(currentNodeBB->getPosition() - tempChildBoundingBox->getHalfSize() + glm::vec3(tempChildBoundingBox->getHalfSize().x * 2.0f * i, tempChildBoundingBox->getHalfSize().y * 2.0f * j, tempChildBoundingBox->getHalfSize().z * 2.0f * k));
							tempChildNode.bbEntity->addComponent<TransformComponent>(tempChildBoundingBox->getPosition(), glm::vec3(0.0f), tempChildBoundingBox->getHalfSize() * 2.0f);
							m_scene->addEntity(tempChildNode.bbEntity);
							tempChildNode.nrOfEntities = 0;
							tempChildNode.parentNode = currentNode;
							currentNode->childNodes.push_back(tempChildNode);

							//Try to put meshes that was in this leaf node in the new child nodes.
							for (int l = 0; l < currentNode->nrOfEntities; l++) {
								if (addEntityRec(currentNode->entities[l], &currentNode->childNodes.back())) {
									//Mesh was successfully added to child. Remove it from this node.
									currentNode->entities.erase(currentNode->entities.begin() + l);
									currentNode->nrOfEntities--;
									l--;
								}
							}
						}
					}
				}

				//Try to add the mesh to newly created child nodes. It gets placed in current node within recursion if the children can not contain it.
				entityAdded = addEntityRec(newEntity, currentNode);
			}
		}
	}

	return entityAdded;
}

bool Octree::removeEntityRec(Entity::SPtr entityToRemove, Node* currentNode) {
	bool entityRemoved = false;

	//Look for mesh in this node
	for (int i = 0; i < currentNode->nrOfEntities; i++) {
		if (currentNode->entities[i] == entityToRemove) {
			//Mesh found - Remove it
			currentNode->entities.erase(currentNode->entities.begin() + i);
			currentNode->nrOfEntities--;
			entityRemoved = true;
			i = currentNode->nrOfEntities;
		}
	}

	if (!entityRemoved) {
		//Mesh was not in this node. Recursively call this function for the children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			if (removeEntityRec(entityToRemove, &currentNode->childNodes[i])) {
				//Mesh was removed by one of the children, break the loop
				entityRemoved = true;
				i = (unsigned int) currentNode->childNodes.size();
			}
		}
	}

	return entityRemoved;
}

void Octree::updateRec(Node* currentNode, std::vector<Entity::SPtr>* entitiesToReAdd) {
	for (int i = 0; i < currentNode->nrOfEntities; i++) {
		if (currentNode->entities[i]->getComponent<BoundingBoxComponent>()->getBoundingBox()->getChange()) { //Entity has changed
			//Re-add the entity to get it in the right node
			Entity::SPtr tempEntity = currentNode->entities[i];
			//First remove the entity from this node to avoid duplicates
			bool removed = removeEntityRec(currentNode->entities[i], currentNode);

			if (removed) {
				i--;
				//Then store the entity to re-add it to the tree in the right node
				entitiesToReAdd->push_back(tempEntity);
			}
		}
	}

	for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
		updateRec(&currentNode->childNodes[i], entitiesToReAdd);
	}
}

void Octree::getCollisionsRec(Entity::SPtr entity, Node* currentNode, std::vector<CollisionInfo>* collisionData) {
	/*
	if (Intersection::aabbWithAabb(entity,&currentNode->bb)) { //Bounding box collides with the current node
		//Check against meshes
		for (int i = 0; i < currentNode->nrOfMeshes; i++) {

			if (Intersection::aabbWithAabb(entity, currentNode->entities[i])) { //Bounding box collides with mesh bounding box
				//Get collision
				for (int j = 0; j < currentNode->entities[i]->getNumberOfVertices(); j += 3) {
					glm::vec3 tempCollision;
					tempCollision = entity->triangleIntersection(currentNode->entities[i]->getVertexPosition(j), currentNode->entities[i]->getVertexPosition(j + 1), currentNode->entities[i]->getVertexPosition(j + 2));

					if (glm::length(tempCollision) > 0.01f) {
						CollisionInfo tempInfo;
						tempInfo.normal = tempCollision;

						for (int k = 0; k < 3; k++) {
							tempInfo.positions[k] = currentNode->entities[i]->getVertexPosition(j + k);
							tempInfo.uvs[k] = currentNode->entities[i]->getVertexUvCoords(j + k);
						}

						//tempInfo.mesh = currentNode->entities[i];

						collisionData->push_back(tempInfo);
					}
				}
			}

		}

		//Check for children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			getCollisionsRec(entity, &currentNode->childNodes[i], collisionData);
		}
	}
	*/
	assert(false); //Not implemented yet
}

float Octree::getRayIntersectionRec(glm::vec3 rayOrigin, glm::vec3 rayDirection, Node* currentNode) {
	float returnValue = -1.0f;

	/*if (Intersection::rayWithAabb(rayOrigin, rayDirection, currentNode->bb) >= 0.0f) { //Ray intersects this node
		//Check against meshes
		for (int i = 0; i < currentNode->nrOfMeshes; i++) {
			if (Intersection::rayWithAabb(rayOrigin, rayDirection, currentNode->entities[i])) {
				//Test for intersection
				for (int j = 0; j < currentNode->entities[i]->getNumberOfVertices(); j += 3) {
					float tempIntersection;
					glm::vec3 triangle[3] = { currentNode->entities[i]->getVertexPosition(j), currentNode->entities[i]->getVertexPosition(j + 1), currentNode->entities[i]->getVertexPosition(j + 2) };
					tempIntersection = Intersection::rayWithTri(rayOrigin, rayDirection, triangle);

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
	}*/

	assert(false); //Not implemented yet

	return returnValue;
}

int Octree::pruneTreeRec(Node* currentNode) {
	int returnValue = 0;

	if (currentNode->childNodes.size() > 0) { //Not a leaf node
		//Call for child nodes
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			returnValue += pruneTreeRec(&currentNode->childNodes[i]);
		}

		if (returnValue == 0) {
			//No entities in any child - Prune the children
			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				ECS::Instance()->destroyEntity(currentNode->childNodes[i].bbEntity);
			}
			currentNode->childNodes.clear();
		}
	}

	returnValue += currentNode->nrOfEntities;

	return returnValue;
}

void Octree::addEntity(Entity::SPtr newEntity) {
	//See if the base node needs to be bigger
	glm::vec3 directionVec = findCornerOutside(newEntity, &m_baseNode);

	if (glm::length(directionVec) != 0.0f) {
		//Entity is outside base node
		//Create bigger base node
		expandBaseNode(directionVec);
		//Recall this function to try to add the mesh again
		addEntity(newEntity);
	}
	else {
		addEntityRec(newEntity, &m_baseNode);
	}
}

void Octree::addEntities(std::vector<Entity::SPtr>* newEntities) {
	for (unsigned int i = 0; i < newEntities->size(); i++) {
		addEntity(newEntities->at(i));
	}
}

void Octree::removeEntity(Entity::SPtr entityToRemove) {
	removeEntityRec(entityToRemove, &m_baseNode);
}

void Octree::removeEntities(std::vector<Entity::SPtr> entitiesToRemove) {
	for (unsigned int i = 0; i < entitiesToRemove.size(); i++) {
		removeEntity(entitiesToRemove[i]);
	}
}

void Octree::update() {
	std::vector<Entity::SPtr> entitiesToReAdd;
	updateRec(&m_baseNode, &entitiesToReAdd);

	addEntities(&entitiesToReAdd);

	entitiesToReAdd.clear();

	pruneTreeRec(&m_baseNode);
}

void Octree::getCollisions(Entity::SPtr entity, std::vector<CollisionInfo>* collisionData) {
	getCollisionsRec(entity, &m_baseNode, collisionData);
}

float Octree::getRayIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection) {
	return getRayIntersectionRec(rayOrigin, rayDirection, &m_baseNode);
}