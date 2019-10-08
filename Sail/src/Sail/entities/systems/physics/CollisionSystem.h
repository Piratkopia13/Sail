#pragma once
#include "..//BaseComponentSystem.h"
#include "..//..//Physics/Octree.h"
#include "..//..//Physics/BoundingBox.h"

class GameDataTracker;

class CollisionSystem final : public BaseComponentSystem {
public:
	CollisionSystem();
	~CollisionSystem();
	
	void provideOctree(Octree* octree);
	void update(float dt) override;

private:
	const bool rayCastCheck(Entity* e, const BoundingBox& boundingBox, float& dt) const;
	void rayCastUpdate(Entity* e, BoundingBox& boundingBox, float& dt);
	const bool collisionUpdate(Entity* e, const float& dt);
	const bool handleCollisions(Entity* e, const std::vector<Octree::CollisionInfo>& collisions, const float& dt);
	void surfaceFromCollision(Entity* e, BoundingBox& boundingBox, const std::vector<Octree::CollisionInfo>& collisions);

private:
	Octree* m_octree = nullptr;
	GameDataTracker* m_gameDataTracker = nullptr;
};