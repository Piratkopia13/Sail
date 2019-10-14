#include "pch.h"
#include "EntityFactory.hpp"

#include "Sail/Application.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/entities/components/Components.h"
#include "Sail/ai/pathfinding/NodeSystem.h"
#include "Sail/ai/states/AttackingState.h"
#include "Sail/ai/states/FleeingState.h"
#include "Sail/ai/states/SearchingState.h"

Entity::SPtr EntityFactory::CreateCandle(const std::string& name, Model* lightModel, Model* bbModel, const glm::vec3& lightPos, size_t lightIndex) {
	//creates light with model and pointlight
	auto e = ECS::Instance()->createEntity(name.c_str());
	e->addComponent<CandleComponent>();
	e->addComponent<ModelComponent>(lightModel);
	e->addComponent<TransformComponent>(lightPos);
	e->addComponent<BoundingBoxComponent>(bbModel);
	e->addComponent<CollidableComponent>();
	e->addComponent<CullingComponent>();
	PointLight pl;
	pl.setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	pl.setPosition(glm::vec3(lightPos.x, lightPos.y + .37f, lightPos.z));
	pl.setAttenuation(.0f, 0.1f, 0.02f);
	pl.setIndex(lightIndex);
	e->addComponent<LightComponent>(pl);

	return e;
}

Entity::SPtr EntityFactory::CreatePlayer(Model* boundingBoxModel, Model* projectileModel, Model* lightModel, unsigned char playerID, size_t lightIndex, glm::vec3 spawnLocation) {

	auto player = ECS::Instance()->createEntity("player");

	// TODO: Only used for AI, should be removed once AI can target player in a better way.
	//m_player = player.get();

	// PlayerComponent is added to this entity to indicate that this is the player playing at this location, not a network connected player
	player->addComponent<PlayerComponent>();

	player->addComponent<TransformComponent>();

	player->addComponent<CullingComponent>();

	player->addComponent<NetworkSenderComponent>(
		Netcode::MessageType::CREATE_NETWORKED_ENTITY,
		Netcode::EntityType::PLAYER_ENTITY,
		playerID);

	// Add physics components and setting initial variables
	player->addComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.0f, -9.8f, 0.0f);
	player->addComponent<SpeedLimitComponent>()->maxSpeed = 6.0f;
	player->addComponent<CollisionComponent>();

	// Give player a bounding box
	player->addComponent<BoundingBoxComponent>(boundingBoxModel);
	player->getComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));

	// Temporary projectile model for the player's gun
	player->addComponent<GunComponent>(projectileModel, boundingBoxModel);

	// Adding audio component and adding all sounds attached to the player entity
	player->addComponent<AudioComponent>();

	Audio::SoundInfo sound{};
	sound.fileName = "../Audio/footsteps_1.wav";
	sound.soundEffectLength = 1.0f;
	sound.volume = 0.5f;
	sound.playOnce = false;
	player->getComponent<AudioComponent>()->defineSound(Audio::SoundType::RUN, sound);

	sound.fileName = "../Audio/jump.wav";
	sound.soundEffectLength = 0.7f;
	sound.playOnce = true;
	player->getComponent<AudioComponent>()->defineSound(Audio::SoundType::JUMP, sound);

	// Create candle for the player
	auto e = CreateCandle("PlayerCandle", lightModel, boundingBoxModel, glm::vec3(0.f, 2.f, 0.f), lightIndex);
	e->addComponent<RealTimeComponent>(); // Player candle will have its position updated each frame
	e->getComponent<CandleComponent>()->setOwner(player->getID());
	player->addChildEntity(e);

	// Set up camera
	//m_cam.setPosition(glm::vec3(1.6f + spawnOffset, 1.8f, 10.f));
	//m_cam.lookAt(glm::vec3(0.f));

	player->getComponent<TransformComponent>()->setStartTranslation(glm::vec3(1.6f, 0.9f, 1.f) + spawnLocation);

	return player;
}

Entity::SPtr EntityFactory::CreateBot(Model* boundingBoxModel, Model* characterModel, const glm::vec3& pos, Model* lightModel, size_t lightIndex, NodeSystem* ns) {

	auto e = ECS::Instance()->createEntity("AiCharacter");
	e->addComponent<ModelComponent>(characterModel);
	e->addComponent<TransformComponent>(pos);
	e->addComponent<BoundingBoxComponent>(boundingBoxModel)->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));
	e->addComponent<CollidableComponent>();
	e->addComponent<MovementComponent>();
	e->addComponent<SpeedLimitComponent>();
	e->addComponent<CollisionComponent>();
	e->addComponent<AiComponent>();
	e->addComponent<CullingComponent>();

	e->addComponent<AudioComponent>();

	Audio::SoundInfo sound{};
	sound.fileName = "../Audio/guitar.wav";
	sound.soundEffectLength = 104.0f;
	sound.volume = 1.0f;
	sound.playOnce = false;
	sound.positionalOffset = { 0.f, 1.2f, 0.f };
	sound.isPlaying = true; // Start playing the sound immediately

	e->getComponent<AudioComponent>()->defineSound(Audio::SoundType::AMBIENT, sound);

	e->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.0f, -9.8f, 0.0f);
	e->getComponent<SpeedLimitComponent>()->maxSpeed = 3.0f;

	e->addComponent<GunComponent>(nullptr, boundingBoxModel);
	auto aiCandleEntity = EntityFactory::CreateCandle("AiCandle", lightModel, boundingBoxModel, glm::vec3(0.f, 2.f, 0.f), lightIndex);

	e->addChildEntity(aiCandleEntity);
	auto fsmComp = e->addComponent<FSMComponent>();

	// =========Create states and transitions===========

	SearchingState* searchState = fsmComp->createState<SearchingState>(ns);
	AttackingState* attackState = fsmComp->createState<AttackingState>();
	fsmComp->createState<FleeingState>(ns);
	
	// TODO: unnecessary to create new transitions for each FSM if they're all identical
	//Attack State
	FSM::Transition* attackToFleeing = SAIL_NEW FSM::Transition;
	attackToFleeing->addBoolCheck(aiCandleEntity->getComponent<CandleComponent>()->getPtrToIsLit(), false);
	FSM::Transition* attackToSearch = SAIL_NEW FSM::Transition;
	attackToSearch->addFloatGreaterThanCheck(attackState->getDistToHost(), 100.0f);
	
	// Search State
	FSM::Transition* searchToAttack = SAIL_NEW FSM::Transition;
	searchToAttack->addFloatLessThanCheck(searchState->getDistToHost(), 100.0f);
	FSM::Transition* searchToFleeing = SAIL_NEW FSM::Transition;
	searchToFleeing->addBoolCheck(aiCandleEntity->getComponent<CandleComponent>()->getPtrToIsLit(), false);
	
	// Fleeing State
	FSM::Transition* fleeingToSearch = SAIL_NEW FSM::Transition;
	fleeingToSearch->addBoolCheck(aiCandleEntity->getComponent<CandleComponent>()->getPtrToIsLit(), true);
	
	fsmComp->addTransition<AttackingState, FleeingState>(attackToFleeing);
	fsmComp->addTransition<AttackingState, SearchingState>(attackToSearch);
	
	fsmComp->addTransition<SearchingState, AttackingState>(searchToAttack);
	fsmComp->addTransition<SearchingState, FleeingState>(searchToFleeing);
	
	fsmComp->addTransition<FleeingState, SearchingState>(fleeingToSearch);
	// =========[END] Create states and transitions===========


	return e;
}

Entity::SPtr EntityFactory::CreateStaticMapObject(std::string name, Model* model, Model* boundingBoxModel, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) {
	auto e = ECS::Instance()->createEntity(name);
	e->addComponent<ModelComponent>(model);
	e->addComponent<TransformComponent>(pos, rot, scale);
	e->addComponent<BoundingBoxComponent>(boundingBoxModel);
	e->addComponent<CollidableComponent>();
	e->addComponent<CullingComponent>();

	return e;
}

Entity::SPtr EntityFactory::CreateProjectile(const glm::vec3& pos, const glm::vec3& velosity, float lifetime, float randomSpreed) {
	auto e = ECS::Instance()->createEntity("projectile");
	glm::vec3 randPos;

	randPos.r = Utils::rnd() * randomSpreed;
	randPos.g = Utils::rnd() * randomSpreed;
	randPos.b = Utils::rnd() * randomSpreed;

	e->addComponent<MetaballComponent>();
	e->addComponent<BoundingBoxComponent>()->getBoundingBox()->setHalfSize(glm::vec3(0.1, 0.1, 0.1));
	e->addComponent<LifeTimeComponent>(lifetime);
	e->addComponent<ProjectileComponent>(10.0f);
	e->addComponent<TransformComponent>(pos + randPos);

	MovementComponent* movement = e->addComponent<MovementComponent>();
	movement->velocity = velosity;
	movement->constantAcceleration = glm::vec3(0.f, -9.8f, 0.f);

	CollisionComponent* collision = e->addComponent<CollisionComponent>();
	collision->drag = 2.0f;
	// NOTE: 0.0f <= Bounciness <= 1.0f
	collision->bounciness = 0.1f;
	collision->padding = 0.16f;

	return e;
}
