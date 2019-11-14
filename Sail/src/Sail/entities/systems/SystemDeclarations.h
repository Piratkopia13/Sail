#pragma once

#include "../components/TransformComponent.h"
#include "../components/ReplayTransformComponent.h"

// Ordered by name

class AiSystem;
class AnimationSystem;
class AnimationChangerSystem;
class AudioSystem;
class BeginEndFrameSystem;
class BoundingboxSubmitSystem;
class CandleHealthSystem;
class CandlePlacementSystem;
class CandleReignitionSystem;
class CollisionSystem;
class EndFrameSystem;
class EntityAdderSystem;
class EntityRemovalSystem;
class GameInputSystem;
class GUISubmitSystem;
class GunSystem;
class KillCamReceiverSystem;
class LevelSystem;
class LifeTimeSystem;
class LightSystem;
class LightListSystem;
class MetaballSubmitSystem;
class MovementPostCollisionSystem;
class MovementSystem;
class NetworkReceiverSystem;
class NetworkSenderSystem;
class OctreeAddRemoverSystem;
class ParticleSystem;
class PlayerSystem;
class PrepareUpdateSystem;
class ProjectileSystem;
class RenderImGuiSystem;
class SpeedLimitSystem;
class SprinklerSystem;
class UpdateBoundingBoxSystem;
class SpotLightSystem;
class SprintingSystem;
class TeamColorSystem;
class CandleThrowingSystem;


// Systems that need duplicate versions for the killcam
template <typename T>
class ModelSubmitSystem;

struct Systems {
	AiSystem*                    aiSystem                    = nullptr;
	AnimationSystem*             animationSystem             = nullptr;
	AnimationChangerSystem*      animationChangerSystem      = nullptr;
	AudioSystem*                 audioSystem                 = nullptr;
	BeginEndFrameSystem*         beginEndFrameSystem         = nullptr;
	BoundingboxSubmitSystem*     boundingboxSubmitSystem     = nullptr;
	CandleHealthSystem*          candleHealthSystem          = nullptr;
	CandlePlacementSystem*       candlePlacementSystem       = nullptr;
	CandleReignitionSystem*      candleReignitionSystem      = nullptr;
	CandleThrowingSystem*		 candleThrowingSystem		 = nullptr;
	CollisionSystem*             collisionSystem             = nullptr;
	EndFrameSystem*              endFrameSystem              = nullptr;
	EntityAdderSystem*           entityAdderSystem           = nullptr;
	EntityRemovalSystem*         entityRemovalSystem         = nullptr;
	GameInputSystem*             gameInputSystem             = nullptr;
	GUISubmitSystem*             guiSubmitSystem             = nullptr;
	GunSystem*                   gunSystem                   = nullptr;
	KillCamReceiverSystem*       killCamReceiverSystem       = nullptr;
	LevelSystem*                 levelSystem                 = nullptr;
	LifeTimeSystem*              lifeTimeSystem              = nullptr;
	LightSystem*                 lightSystem                 = nullptr;
	LightListSystem*             lightListSystem             = nullptr;
	MetaballSubmitSystem*        metaballSubmitSystem        = nullptr;
	MovementPostCollisionSystem* movementPostCollisionSystem = nullptr;
	MovementSystem*              movementSystem              = nullptr;
	NetworkReceiverSystem*       networkReceiverSystem       = nullptr;
	NetworkSenderSystem*         networkSenderSystem         = nullptr;
	OctreeAddRemoverSystem*      octreeAddRemoverSystem      = nullptr;
	ParticleSystem*              particleSystem              = nullptr;
	PlayerSystem*                playerSystem                = nullptr;
	PrepareUpdateSystem*         prepareUpdateSystem         = nullptr;
	ProjectileSystem*            projectileSystem            = nullptr;
	RenderImGuiSystem*           renderImGuiSystem           = nullptr;
	SpeedLimitSystem*            speedLimitSystem            = nullptr;
	SpotLightSystem*             spotLightSystem             = nullptr;
	TeamColorSystem*             teamColorSystem             = nullptr;
	SprinklerSystem*             sprinklerSystem             = nullptr;
	UpdateBoundingBoxSystem*     updateBoundingBoxSystem     = nullptr;
	SprintingSystem*             sprintingSystem             = nullptr;


	// Systems that need duplicate versions for the killcam
	ModelSubmitSystem<TransformComponent>*       modelSubmitSystem        = nullptr;
	ModelSubmitSystem<ReplayTransformComponent>* killCamModelSubmitSystem = nullptr;
};