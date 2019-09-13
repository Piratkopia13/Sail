//#pragma once
//
//#include "Component.h"
//
//#include "../../patterns/Node.h"
//#include "TransformMatrixComponent.h"
//
//
//// TODO: Rewrite this more cleanly
//
//constexpr int prevInd(int ind) {
//	return (ind + SNAPSHOT_BUFFER_SIZE - 1) % SNAPSHOT_BUFFER_SIZE;
//}
//
//class TransformDataComponent : public Component, public Node<TransformDataComponent> {
//public:
//	SAIL_COMPONENT
//
//
//	//TransformDataComponent(const glm::vec3& translation, TransformDataComponent* parent = nullptr)
//	//	: m_translation(translation), Node(this, parent) {}
//
//	//TransformDataComponent(TransformDataComponent* parent) : Node(this, parent) {}
//
//	TransformDataComponent(
//			TransformMatrixComponent* matrixComponent,
//			const glm::vec3& translation = { 0.0f, 0.0f, 0.0f },
//			const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f },
//			const glm::vec3& scale = { 1.0f, 1.0f, 1.0f })
//		: Component(),
//		Node(this),
//		m_matrixComponent(matrixComponent)
//	{
//		for (auto &t : m_snapshots) {
//			t.m_translation = translation;
//			t.m_rotation = rotation;
//			t.m_scale = scale;
//		}
//		for (auto& b : m_dataUpdated) {
//			b = true;
//		}
//	
//	}
//
//
//	void setTranslation(const UINT ind, const glm::vec3& translation) { 
//		m_snapshots[ind].m_translation = translation; 
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void setTranslation(const UINT ind, const float x, const float y, const float z) {
//		m_snapshots[ind].m_translation = glm::vec3(x, y, z);
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//	
//	void setRotation(const UINT ind, const glm::vec3& rotation) { 
//		m_snapshots[ind].m_rotation = rotation; 
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void setScale(const UINT ind, const float scale) {
//		m_snapshots[ind].m_scale = glm::vec3(scale, scale, scale);
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void setScale(const UINT ind, const float x, const float y, const float z) {
//		m_snapshots[ind].m_scale = glm::vec3(x, y, z);
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void setScale(const UINT ind, const glm::vec3& scale) { 
//		m_snapshots[ind].m_scale = scale; 
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void translate(const UINT ind, const glm::vec3& move) {
//		m_snapshots[ind].m_translation += move;
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void translate(const UINT ind, const float x, const float y, const float z) {
//		m_snapshots[ind].m_translation += glm::vec3(x, y, z);
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void rotateAroundX(const UINT ind, const float radians) {
//		m_snapshots[ind].m_rotation.x += radians;
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void rotateAroundY(const UINT ind, const float radians) {
//		m_snapshots[ind].m_rotation.y += radians;
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	void rotateAroundZ(const UINT ind, const float radians) {
//		m_snapshots[ind].m_rotation.z += radians;
//		m_dataUpdated[ind] = true;
//		treeNeedsUpdating();
//	}
//
//	// alpha = [0,1]
//	// alpha of 1 is the most recent snapshot and alpha of 0 is the one before that
//	glm::mat4 getMatrixFromData(const UINT ind, const float alpha) {
//		if (m_dataUpdated[ind]) {
//			m_matrixComponent->updateLocalMatrix(
//				m_snapshots[ind].m_translation, 
//				m_snapshots[ind].m_rotation, 
//				m_snapshots[ind].m_scale);
//			m_dataUpdated[ind] = false;
//		}
//		/*if (getParentUpdated() || !hasParent()) {
//			m_matrixComponent->updateMatrix();
//			setParentUpdated(false);
//		}*/
//		if (getParentUpdated() && hasParent()) {
//			m_matrixComponent->updateMatrixWithParent(getParentMatrix(ind));
//			setParentUpdated(false);
//		} else if (!hasParent()) {
//			m_matrixComponent->updateMatrix();
//			setParentUpdated(false);
//		}
//
//		return m_matrixComponent->getTransformMatrix();
//	}
//
//	// NOTE: Should be done at the beggining of each update
//	void copyDataFromPrevUpdate(const UINT ind) {
//		m_snapshots[ind] = m_snapshots[prevInd(ind)];
//	}
//	
//
//	const glm::vec3& getTranslation(const UINT ind) const { return m_snapshots[ind].m_translation; }
//	const glm::vec3& getRotation(const UINT ind) const { return m_snapshots[ind].m_rotation; }
//	const glm::vec3& getScale(const UINT ind) const { return m_snapshots[ind].m_scale; }
//	const bool wasUpdatedThisTick(const UINT ind) const { return m_dataUpdated[ind]; }
//
//	// called once the positions have been used to update relevant matrices
//	void dataProcessed(const UINT ind) { m_dataUpdated[ind] = true; }
//private:
//	glm::mat4 getParentMatrix(const UINT ind) const {
//		return m_parent->getDataPtr()->getMatrixFromData(ind, 1.0);
//	}
//
//
//	// Written to in update loop
//	//glm::vec3 m_translation;
//	//glm::vec3 m_rotation;
//	//glm::vec3 m_scale;
//
//
//	struct TransformSnapshot {
//		glm::vec3 m_translation;
//		glm::vec3 m_rotation;
//		glm::vec3 m_scale;
//	};
//
//	TransformSnapshot m_snapshots[SNAPSHOT_BUFFER_SIZE];
//	bool m_dataUpdated[SNAPSHOT_BUFFER_SIZE];
//
//	TransformMatrixComponent* m_matrixComponent;
//
//};