#pragma once

// Ordered by name

class AiSystem;
class AnimationSystem;
class AudioSystem;
class CandleSystem;
class CollisionSystem;
class EndFrameSystem;
class EntityAdderSystem;
class EntityRemovalSystem;
class GameInputSystem;
class GunSystem;
class HitboxSubmitSystem;
class LevelGeneratorSystem;
class LifeTimeSystem;
class LightSystem;
class MovementPostCollisionSystem;
class MovementSystem;
class NetworkReceiverSystem;
class NetworkSenderSystem;
class OctreeAddRemoverSystem;
class PrepareUpdateSystem;
class ProjectileSystem;
class RenderSystem;
class SpeedLimitSystem;
class UpdateBoundingBoxSystem;

struct Systems {
	AiSystem* aiSystem = nullptr;
	AnimationSystem* animationSystem = nullptr;
	AudioSystem* audioSystem = nullptr;
	CandleSystem* candleSystem = nullptr;
	CollisionSystem* collisionSystem = nullptr;
	EndFrameSystem* endFrameSystem = nullptr;
	EntityAdderSystem* entityAdderSystem = nullptr;
	EntityRemovalSystem* entityRemovalSystem = nullptr;
	GameInputSystem* gameInputSystem = nullptr;
	GunSystem* gunSystem = nullptr;
	HitboxSubmitSystem* hitboxSubmitSystem = nullptr;
	LevelGeneratorSystem* levelGeneratorSystem = nullptr;
	LifeTimeSystem* lifeTimeSystem = nullptr;
	LightSystem* lightSystem = nullptr;
	MovementPostCollisionSystem* movementPostCollisionSystem = nullptr;
	MovementSystem* movementSystem = nullptr;
	NetworkReceiverSystem* networkReceiverSystem = nullptr;
	NetworkSenderSystem* networkSenderSystem = nullptr;
	OctreeAddRemoverSystem* octreeAddRemoverSystem = nullptr;
	PrepareUpdateSystem* prepareUpdateSystem = nullptr;
	ProjectileSystem* projectileSystem = nullptr;
	RenderSystem* renderSystem = nullptr;
	SpeedLimitSystem* speedLimitSystem = nullptr;
	UpdateBoundingBoxSystem* updateBoundingBoxSystem = nullptr;
};