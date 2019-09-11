#pragma once

#include "Component.h"

#include "../../patterns/Node.h"
#include "TransformMatrixComponent.h"


// TODO: rewrite TransformComponent to make it leaner, use this component for interpolation eventually
// keep the matrixes in it

class TransformDataComponent : public Component, public Node<TransformDataComponent> {
public:
	SAIL_COMPONENT


	//TransformDataComponent(const glm::vec3& translation, TransformDataComponent* parent = nullptr)
	//	: m_translation(translation), Node(this, parent) {}

	//TransformDataComponent(TransformDataComponent* parent) : Node(this, parent) {}

	TransformDataComponent(
			TransformMatrixComponent* matrixComponent,
			const glm::vec3& translation = { 0.0f, 0.0f, 0.0f },
			const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f },
			const glm::vec3& scale = { 1.0f, 1.0f, 1.0f })
		: m_translation(translation),
		m_rotation(rotation),
		m_scale(scale),
		Component(),
		Node(this),
		m_matrixComponent(matrixComponent)
	{}


	void setTranslation(const glm::vec3& translation) { 
		m_translation = translation; 
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setTranslation(const float x, const float y, const float z) {
		m_translation = glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}
	
	void setRotation(const glm::vec3& rotation) { 
		m_rotation = rotation; 
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setScale(const float scale) {
		m_scale = glm::vec3(scale, scale, scale);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setScale(const float x, const float y, const float z) {
		m_scale = glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setScale(const glm::vec3& scale) { 
		m_scale = scale; 
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void translate(const glm::vec3& move) {
		m_translation += move;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void translate(const float x, const float y, const float z) {
		m_translation += glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundX(const float radians) {
		m_rotation.x += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundY(const float radians) {
		m_rotation.y += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundZ(const float radians) {
		m_rotation.z += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	glm::mat4 getMatrixFromData() {
		if (m_dataUpdated) {
			m_matrixComponent->updateLocalMatrix(m_translation, m_rotation, m_scale);
			m_dataUpdated = false;
		}
		/*if (getParentUpdated() || !hasParent()) {
			m_matrixComponent->updateMatrix();
			setParentUpdated(false);
		}*/
		if (getParentUpdated() && hasParent()) {
			m_matrixComponent->updateMatrixWithParent(getParentMatrix());
			setParentUpdated(false);
		} else if (!hasParent()) {
			m_matrixComponent->updateMatrix();
			setParentUpdated(false);
		}

		return m_matrixComponent->getTransformMatrix();
	}


	

	const glm::vec3& getTranslation() const { return m_translation; }
	const glm::vec3& getRotation() const { return m_rotation; }
	const glm::vec3& getScale() const { return m_scale; }
	const bool wasUpdatedThisTick() const { return m_dataUpdated; }

	// called once the positions have been used to update relevant matrices
	void dataProcessed() { m_dataUpdated = true; }
private:
	glm::mat4 getParentMatrix() const {
		return m_parent->getDataPtr()->getMatrixFromData();
	}




	TransformMatrixComponent* m_matrixComponent;

	// Written to in update loop
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	bool m_dataUpdated;
};