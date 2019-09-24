// TODO: REMOVE

#pragma once
#include "Transform.h"

// Forward declares
class TransformComponent;
class ModelComponent;
class Model;

class PerUpdateRenderObject {
public:
	explicit PerUpdateRenderObject();
	explicit PerUpdateRenderObject(TransformComponent* gameObject, ModelComponent* model);
	explicit PerUpdateRenderObject(Transform* transform, Model* model);
	virtual ~PerUpdateRenderObject();

	void createSnapShotFromGameObject(TransformComponent* object);
	void createSnapShotFromGameObject(Transform* object);

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

	std::vector<PerUpdateRenderObject*> m_children;
private:
	void updateLocalMatrix();
	void updateMatrix();

	void addChild(PerUpdateRenderObject* transform);
	void removeChild(PerUpdateRenderObject* transform);
};