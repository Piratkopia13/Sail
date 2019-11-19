#pragma once
#include "..//BaseComponentSystem.h"
#include "..//..//Physics/Octree.h"
#include "..//..//Physics/BoundingBox.h"

class CollisionSystem final : public BaseComponentSystem {
public:
	CollisionSystem();
	~CollisionSystem();
	
	void provideOctree(Octree* octree);
	void update(float dt);

private:
	bool collisionUpdatePart(float dt, size_t start, size_t end);
	bool surfaceFromCollisionPart(float dt, size_t start, size_t end);
	bool rayCastCollisionPart(float dt, size_t start, size_t end);
	
	const bool rayCastCheck(Entity* e, const BoundingBox& boundingBox, float& dt) const;
	void rayCastUpdate(Entity* e, BoundingBox& boundingBox, float& dt);
	void collisionUpdate(Entity* e, const float dt);
	const bool handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt);
	const bool handleRagdollCollisions(Entity* e, const float dt);
	void gatherCollisionInformation(Entity* e, const BoundingBox* boundingBox, std::vector<Octree::CollisionInfo>& collisions, std::vector<Octree::CollisionInfo>& trueCollisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
	void updateVelocityVec(Entity* e, glm::vec3& velocity, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
	void surfaceFromCollision(Entity* e, std::vector<Octree::CollisionInfo>& collisions);

private:
	Octree* m_octree;
};