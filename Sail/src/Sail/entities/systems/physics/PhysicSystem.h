#pragma once
#include "..//BaseComponentSystem.h"
#include "../../Physics/Physics.h"

class GameDataTracker;
class PhysicsComponent;
class TransformComponent;
class BoundingBoxComponent;
class CollisionSpheresComponent;

class PhysicSystem final : public BaseComponentSystem
{
public:
	PhysicSystem();
	~PhysicSystem();

	void provideOctree(Octree* octree);

	void update(float dt) override;

private:
	Octree* m_octree = nullptr;
	GameDataTracker* m_gameDataTracker = nullptr;

	const bool rayCastCheck(Entity* e, PhysicsComponent* physicsComp, BoundingBox* boundingBox, float& dt);
	void rayCastUpdate(Entity* e, PhysicsComponent* physicsComp, BoundingBox* boundingBox, TransformComponent* transform, float& dt);
	const bool collisionUpdate(Entity* thisPhysicalObject, PhysicsComponent* physicsComp, const float& dt);
	const bool handleCollisions(Entity* e, PhysicsComponent* physicsComp, const std::vector<Octree::CollisionInfo>& collisions, const float& dt);
	void surfaceFromCollision(BoundingBoxComponent* bbComp, CollisionSpheresComponent* csc, PhysicsComponent* physics, TransformComponent* transform, const std::vector<Octree::CollisionInfo>& collisions);


	float findIntersectionDepth(BoundingBoxComponent* bbComp, CollisionSpheresComponent* csc, const Octree::CollisionInfo& collision);
};