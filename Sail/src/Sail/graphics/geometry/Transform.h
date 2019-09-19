#pragma once
#include <glm/vec3.hpp>

// forward declaration
class PerUpdateRenderObject;

// Structs for storing transform data from two consecutive updates
// so that they can be interpolated between.
// Should be optimized more in the future.
struct TransformSnapshot {
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::quat m_rotationQuat;
	glm::vec3 m_scale;
	glm::vec3 m_forward;
	glm::vec3 m_right;
	glm::vec3 m_up;

};

struct TransformFrame {
	TransformSnapshot m_current;
	TransformSnapshot m_previous;

	bool m_updatedDirections;
};

class Transform {

public:
	explicit Transform(Transform* parent);
	//Transform(TransformSnapshot current, TransformSnapshot prev);
	Transform(const glm::vec3& translation, Transform* parent = nullptr);
	Transform(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f },
		const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f },
		const glm::vec3& scale = { 1.0f, 1.0f, 1.0f },
		Transform* parent = nullptr);
	virtual ~Transform();

	void setParent(Transform* parent);
	void removeParent();

	void prepareUpdate();
	TransformSnapshot getCurrentTransformState() const;
	TransformSnapshot getPreviousTransformState() const;

	TransformFrame getTransformFrame() const;

	void setStartTranslation(const glm::vec3& translation);

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

	/* Forward should always be a normalized vector */
	void setForward(const glm::vec3& forward);

	PerUpdateRenderObject* getRenderTransform() const;
	Transform* getParent() const;

	const glm::vec3& getTranslation() const;
	const glm::vec3& getRotations() const;
	const glm::vec3& getScale() const;

	const glm::vec3 getInterpolatedTranslation(float alpha) const;

	//const glm::vec3& getForward();
	//const glm::vec3& getRight();
	//const glm::vec3& getUp();
private:
	TransformFrame m_data;

	Transform* m_parent = nullptr;

	std::vector<Transform*> m_children;
private:
	void treeNeedsUpdating();
	void addChild(Transform* transform);
	void removeChild(Transform* transform);
};