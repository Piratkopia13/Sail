#pragma once
#include "TransformCommon.h"

// forward declaration
class RenderTransform;

class GameTransform {

public:
	explicit GameTransform(GameTransform* parent);
	//GameTransform(TransformSnapshot current, TransformSnapshot prev);
	GameTransform(const glm::vec3& translation, GameTransform* parent = nullptr);
	GameTransform(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, 
		const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, 
		const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, 
		GameTransform* parent = nullptr);
	virtual ~GameTransform();

	void setParent(GameTransform* parent);
	void removeParent();

	void prepareUpdate();
	TransformSnapshot getCurrentTransformState() const;
	TransformSnapshot getPreviousTransformState() const;
	//GameTransform* getTransformSnapshot() const;
	//void copySnapshotFromObject(GameTransform* object);

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


	GameTransform* getParent() const;

	RenderTransform* getRenderTransform() const;

	const glm::vec3& getTranslation() const;
	const glm::vec3& getRotations() const;
	const glm::vec3& getScale() const;
	//const glm::vec3& getForward();
	//const glm::vec3& getRight();
	//const glm::vec3& getUp();
private:
	TransformFrame m_data;

	GameTransform* m_parent = nullptr;

	// GameTransform is not responsible for the destruction of its children
	// since they are connected to their own entities.
	std::vector<GameTransform*> m_children;
private:
	void treeNeedsUpdating();
	void addChild(GameTransform* transform);
	void removeChild(GameTransform* transform);


	//static constexpr int prevInd(int ind) {
	//	return (ind + SNAPSHOT_BUFFER_SIZE - 1) % SNAPSHOT_BUFFER_SIZE;
	//}

	//// first frame is 0 and it continues from there, integer overflow isn't a problem unless
	//// you leave the game running for like a year or two.
	//// Note: atomic since it's written to in every update and read from in every update and render
	//static std::atomic_uint s_frameIndex;

	//// the index in the snapshot buffer that is used in the update loop on the CPU.
	//// [0, SNAPSHOT_BUFFER_SIZE-1]
	//// Note: Updated once at the start of update and read-only in update so no atomics needed
	//static UINT s_updateIndex;


	//// If CPU update is working on index 3 then prepare render will safely interpolate between
	//// index 1 and 2 without any data races
	//// Note: Updated once at the start of render and read-only in render so no atomics needed
	//static UINT s_renderIndex;
};