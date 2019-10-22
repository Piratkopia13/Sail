#pragma once

// Ordered by name

class AiSystem;
class AnimationInitSystem;
class AnimationSystem;
class AudioSystem;
class BeginEndFrameSystem;
class BoundingboxSubmitSystem;
class CandleSystem;
class CollisionSystem;
class EndFrameSystem;
class EntityAdderSystem;
class EntityRemovalSystem;
class GameInputSystem;
class GunSystem;
class LevelGeneratorSystem;
class LifeTimeSystem;
class LightSystem;
class LightListSystem;
class MetaballSubmitSystem;
class ModelSubmitSystem;
class MovementPostCollisionSystem;
class MovementSystem;
class NetworkReceiverSystem;
class NetworkSenderSystem;
class OctreeAddRemoverSystem;
class PrepareUpdateSystem;
class ProjectileSystem;
class RealTimeModelSubmitSystem;
class RenderImGuiSystem;
class SpeedLimitSystem;
class UpdateBoundingBoxSystem;

class NetworkReceiverSystem;

struct Systems {
	AiSystem* aiSystem = nullptr;
	AnimationInitSystem* animationInitSystem = nullptr;
	AnimationSystem* animationSystem = nullptr;
	AudioSystem* audioSystem = nullptr;
	BeginEndFrameSystem* beginEndFrameSystem = nullptr;
	BoundingboxSubmitSystem* boundingboxSubmitSystem = nullptr;
	CandleSystem* candleSystem = nullptr;
	CollisionSystem* collisionSystem = nullptr;
	EndFrameSystem* endFrameSystem = nullptr;
	EntityAdderSystem* entityAdderSystem = nullptr;
	EntityRemovalSystem* entityRemovalSystem = nullptr;
	GameInputSystem* gameInputSystem = nullptr;
	GunSystem* gunSystem = nullptr;
	LevelGeneratorSystem* levelGeneratorSystem = nullptr;
	LifeTimeSystem* lifeTimeSystem = nullptr;
	LightSystem* lightSystem = nullptr;
	LightListSystem* lightListSystem = nullptr;
	MetaballSubmitSystem* metaballSubmitSystem = nullptr;
	ModelSubmitSystem* modelSubmitSystem = nullptr;
	MovementPostCollisionSystem* movementPostCollisionSystem = nullptr;
	MovementSystem* movementSystem = nullptr;
	NetworkReceiverSystem* networkReceiverSystem = nullptr;
	NetworkSenderSystem* networkSenderSystem = nullptr;
	OctreeAddRemoverSystem* octreeAddRemoverSystem = nullptr;
	PrepareUpdateSystem* prepareUpdateSystem = nullptr;
	ProjectileSystem* projectileSystem = nullptr;
	RealTimeModelSubmitSystem* realTimeModelSubmitSystem = nullptr;
	RenderImGuiSystem* renderImGuiSystem = nullptr;
	SpeedLimitSystem* speedLimitSystem = nullptr;
	UpdateBoundingBoxSystem* updateBoundingBoxSystem = nullptr;
};