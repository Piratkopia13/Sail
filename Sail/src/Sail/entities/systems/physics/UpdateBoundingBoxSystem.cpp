#include "pch.h"
#include "UpdateBoundingBoxSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/ModelComponent.h"
#include "Sail/entities/components/RagdollComponent.h"

#include "Sail/graphics/geometry/Model.h"

UpdateBoundingBoxSystem::UpdateBoundingBoxSystem() : BaseComponentSystem() {
	registerComponent<BoundingBoxComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<ModelComponent>(false, true, true);
	registerComponent<RagdollComponent>(false, true, true);
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
		glm::vec3 minPositions(9999999.0f), maxPositions(-9999999.0f);

		//Recalculate min and max
		for (unsigned int i = 0; i < model->getModel()->getNumberOfMeshes(); i++) {
			const Mesh::Data& meshData = model->getModel()->getMesh(i)->getData();
			for (unsigned int j = 0; j < meshData.numVertices; j++) {
				glm::vec3 posAfterTransform = glm::vec3(transform->getMatrixWithUpdate() * glm::vec4(meshData.positions[j].vec, 1.0f));
				checkDistances(minPositions, maxPositions, posAfterTransform);
			}
		}
		boundingBox->getBoundingBox()->setHalfSize((maxPositions - minPositions) * 0.5f);
		boundingBox->getBoundingBox()->setPosition(minPositions + boundingBox->getBoundingBox()->getHalfSize());
	}
	else {
		boundingBox->getBoundingBox()->setPosition(transform->getTranslation() + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	}

	/*transform->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	transform->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);*/
	boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
}

void UpdateBoundingBoxSystem::recalculateBoundingBoxPosition(Entity* e) {
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	glm::mat4 transformationMatrix = transform->getMatrixWithUpdate();
	boundingBox->getBoundingBox()->setPosition(glm::vec3(transformationMatrix[3]) + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	/*transform->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	transform->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);*/
	boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
}

void UpdateBoundingBoxSystem::updateRagdollBoundingBoxes(Entity* e) {
	RagdollComponent* ragdollComp = e->getComponent<RagdollComponent>();
	TransformComponent* transComp = e->getComponent<TransformComponent>();

	for (size_t i = 0; i < ragdollComp->contactPoints.size(); i++) {
		ragdollComp->contactPoints[i].boundingBox.setPosition(glm::vec3(transComp->getMatrixWithUpdate() * glm::vec4(ragdollComp->contactPoints[i].localOffSet, 1.0f)));
		ragdollComp->contactPoints[i].transform.setTranslation(ragdollComp->contactPoints[i].boundingBox.getPosition() - glm::vec3(0.0f, ragdollComp->contactPoints[i].boundingBox.getHalfSize().y, 0.0f));
		ragdollComp->contactPoints[i].transform.setScale(ragdollComp->contactPoints[i].boundingBox.getHalfSize() * 2.0f);
	}
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
			if (change > 1 && !e->getComponent<BoundingBoxComponent>()->isStatic) {
				recalculateBoundingBoxFully(e);
			} 
			else if (change > 0) {
				recalculateBoundingBoxPosition(e);
			}

			if (e->hasComponent<RagdollComponent>()) {
				recalculateBoundingBoxFully(e);
				updateRagdollBoundingBoxes(e);
			}
		}
	}
}
