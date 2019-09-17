#pragma once

#include "pch.h" // for SNAPSHOT_BUFFER_IND
#include "TransformCommon.h"

// Forward declare
class GameTransformComponent;
class ModelComponent;
class Model;

// TODO: Rename to something more fitting like objectFramePacket or something
class RenderTransform {
public:
	explicit RenderTransform();
	explicit RenderTransform(GameTransformComponent* gameObject, ModelComponent* model);
	//RenderTransform(TransformFrame frame);
	//RenderTransform(const glm::vec3& translation, RenderTransform* parent = nullptr);
	//RenderTransform(
	//	const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, 
	//	const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, 
	//	const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, 
	//	RenderTransform* parent = nullptr);
	virtual ~RenderTransform();

	void setParent(RenderTransform* parent);
	void removeParent();

	void createSnapShotFromGameObject(GameTransformComponent* object);


	void setMatrix(const glm::mat4& newMatrix);


	glm::mat4 getMatrix(float alpha = 1.0f);
	//glm::mat4 getLocalMatrix();

	RenderTransform* getParent() const;
	Model* getModel() const;
private:
	Model* m_model = nullptr;
	
	TransformFrame m_data;

	glm::mat4 m_rotationMatrix;
	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;

	RenderTransform* m_parent = nullptr;

	// RenderTransform IS responsible for the destruction of its children
	// since they are local copies and not saved anywhere else
	std::vector<RenderTransform*> m_children;


private:
	void updateLocalMatrix();
	void updateMatrix();
	void treeNeedsUpdating();
	void addChild(RenderTransform* transform);
	void removeChild(RenderTransform* transform);
};