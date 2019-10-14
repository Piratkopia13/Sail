#include "pch.h"
#include "UpdateBoundingBoxSystem.h"
#include "../../Entity.h"
#include "../../components/TransformComponent.h"
#include "../../components/BoundingBoxComponent.h"
#include "../../components/ModelComponent.h"

#include "../../../graphics/geometry/Model.h"

UpdateBoundingBoxSystem::UpdateBoundingBoxSystem() : BaseComponentSystem() {
	registerComponent<BoundingBoxComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<ModelComponent>(false, true, true);
}

UpdateBoundingBoxSystem::~UpdateBoundingBoxSystem() {
}

void UpdateBoundingBoxSystem::checkDistances(glm::vec3& minVec, glm::vec3& maxVec, const glm::vec3& testVec) {
	if (testVec.x > maxVec.x) {
		maxVec.x = testVec.x;
	}
	if (testVec.x < minVec.x) {
		minVec.x = testVec.x;
	}
	if (testVec.y > maxVec.y) {
		maxVec.y = testVec.y;
	}
	if (testVec.y < minVec.y) {
		minVec.y = testVec.y;
	}
	if (testVec.z > maxVec.z) {
		maxVec.z = testVec.z;
	}
	if (testVec.z < minVec.z) {
		minVec.z = testVec.z;
	}
}

void UpdateBoundingBoxSystem::recalculateBoundingBoxFully(Entity* e) {
	ModelComponent* model = e->getComponent<ModelComponent>();
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	if (model) {
		glm::vec3 minPositions(INFINITY), maxPositions(-INFINITY);

		//Recalculate min and max
		for (unsigned int i = 0; i < model->getModel()->getNumberOfMeshes(); i++) {
			const Mesh::Data& meshData = model->getModel()->getMesh(i)->getData();
			for (unsigned int j = 0; j < meshData.numVertices; j++) {
				glm::vec3 posAfterTransform = glm::vec3(transform->getMatrix() * glm::vec4(meshData.positions[j].vec, 1.0f));
				checkDistances(minPositions, maxPositions, posAfterTransform);
			}
		}

		boundingBox->getBoundingBox()->setHalfSize((maxPositions - minPositions) * 0.5f);
		boundingBox->getBoundingBox()->setPosition(minPositions + boundingBox->getBoundingBox()->getHalfSize());
	}
	else {
		boundingBox->getBoundingBox()->setPosition(transform->getTranslation() + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	}

	boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
}

void UpdateBoundingBoxSystem::recalculateBoundingBoxPosition(Entity* e) {
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	glm::mat4 transformationMatrix = transform->getMatrix();
	boundingBox->getBoundingBox()->setPosition(glm::vec3(transformationMatrix[3]) + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
}

bool UpdateBoundingBoxSystem::addEntity(Entity* entity) {
	if (BaseComponentSystem::addEntity(entity)) {
		recalculateBoundingBoxFully(entity);
		return true;
	}
	return false;
}

void UpdateBoundingBoxSystem::update(float dt) {
	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		if (transform) {
			int change = transform->getChange();
			if (change > 1) {
				recalculateBoundingBoxFully(e);
			} 
			else if (change > 0) {
				recalculateBoundingBoxPosition(e);
			}
		}
	}
}
