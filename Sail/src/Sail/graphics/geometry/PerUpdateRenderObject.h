#pragma once

#include "pch.h" // for SNAPSHOT_BUFFER_IND
#include "Transform.h"

// Forward declare
class TransformComponent;
class ModelComponent;
class Model;

// TODO: Rename to something more fitting like objectFramePacket or something
class PerUpdateRenderObject {
public:
	explicit PerUpdateRenderObject();
	explicit PerUpdateRenderObject(TransformComponent* gameObject, ModelComponent* model);
	virtual ~PerUpdateRenderObject();

	void createSnapShotFromGameObject(TransformComponent* object);

	void setMatrix(const glm::mat4& newMatrix);
	glm::mat4 getMatrix(float alpha = 1.0f);
	Model* getModel() const;

	void setParent(PerUpdateRenderObject* parent);
	void removeParent();
	PerUpdateRenderObject* getParent() const;
private:
	Model* m_model = nullptr;
	TransformFrame m_data;

	glm::mat4 m_rotationMatrix;
	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;


	PerUpdateRenderObject* m_parent = nullptr;

	// RenderTransform IS responsible for the destruction of its children
	// since they are local copies and not saved anywhere else
	std::vector<PerUpdateRenderObject*> m_children;
private:
	void updateLocalMatrix();
	void updateMatrix();

	void treeNeedsUpdating();
	void addChild(PerUpdateRenderObject* transform);
	void removeChild(PerUpdateRenderObject* transform);
};