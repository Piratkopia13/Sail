#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Transform {

public:
	explicit Transform(Transform* parent);
	Transform(const glm::vec3& translation, Transform* parent = nullptr);
	Transform(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, Transform* parent = nullptr);
	virtual ~Transform();

	void setParent(Transform* parent);
	void removeParent();

	void translate(const glm::vec3& move);
	void translate(const float x, const float y, const float z);
	
	void scale(const float factor);
	void scale(const glm::vec3& scale);

	//In radians
	void rotate(const glm::vec3& rotation);
	//In radians
	void rotate(const float x, const float y, const float z);
	void rotateAroundX(const float radians);
	void rotateAroundY(const float radians);
	void rotateAroundZ(const float radians);


	void setTranslation(const glm::vec3& translation);
	void setTranslation(const float x, const float y, const float z);
	
	void setRotations(const glm::vec3& rotations);
	void setRotations(const float x, const float y, const float z);
	void setScale(const float scale);
	void setScale(const float x, const float y, const float z);
	void setScale(const glm::vec3& scale);

	void setMatrix(const glm::mat4& newMatrix);


	const glm::vec3& getTranslation() const;
	const glm::vec3& getRotations() const;
	const glm::vec3& getScale() const;

	glm::mat4 getMatrix();
	glm::mat4 getLocalMatrix();

private:

	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;

	bool m_matNeedsUpdate;
	bool m_parentUpdated;

	Transform* m_parent;
	std::vector<Transform*> m_children;
private:
	void updateLocalMatrix();
	void updateMatrix();
	void treeNeedsUpdating();
	void addChild(Transform* transform);
	void removeChild(Transform* transform);
};