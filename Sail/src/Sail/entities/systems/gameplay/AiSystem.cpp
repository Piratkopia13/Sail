#include "pch.h"
#include "AiSystem.h"
#include "../../components/AiComponent.h"
#include "../../components/PhysicsComponent.h"
#include "../../components/TransformComponent.h"
#include "../../components/CandleComponent.h"
#include "../../components/GunComponent.h"
#include "Sail/ai/pathfinding/NodeSystem.h"

#include "../../ECS.h"
#include "../../components/BoundingBoxComponent.h"
#include "../physics/UpdateBoundingBoxSystem.h"
#include "../physics/OctreeAddRemoverSystem.h"
#include "Sail/utils/Utils.h"
#include "../Physics/Octree.h"
#include "Sail/Application.h"
#include "../Physics/Intersection.h"
#include "../Physics/Physics.h"

AiSystem::AiSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
	readBits |= TransformComponent::BID;
	writeBits |= TransformComponent::BID;
	requiredComponentTypes.push_back(PhysicsComponent::ID);
	readBits |= PhysicsComponent::BID;
	writeBits |= PhysicsComponent::BID;
	requiredComponentTypes.push_back(AiComponent::ID);
	readBits |= AiComponent::BID;
	writeBits |= AiComponent::BID;

	m_nodeSystem = std::make_unique<NodeSystem>();
}

AiSystem::~AiSystem() {}

#ifdef _DEBUG_NODESYSTEM
void AiSystem::initNodeSystem(Model* bbModel, Octree* octree, Shader* shader, Scene* scene) {
	m_nodeSystem->setDebugModelAndScene(shader, scene);
	initNodeSystem(bbModel, octree);
}
#endif

void AiSystem::initNodeSystem(Model* bbModel, Octree* octree) {

	std::vector<NodeSystem::Node> nodes;
	std::vector<std::vector<unsigned int>> connections;

	m_octree = octree;

	std::vector<unsigned int> conns;
	int x_max = 15;
	int z_max = 15;
	int x_cur = 0;
	int z_cur = 0;
	int size = x_max * z_max;

	int padding = 2;
	float offsetX = x_max * padding * 0.5f;
	float offsetZ = z_max * padding * 0.5f;
	float offsetY = 0;
	bool* walkable = SAIL_NEW bool[size];

	auto e = ECS::Instance()->createEntity("DeleteMeFirstFrameDummy");
	e->addComponent<BoundingBoxComponent>(bbModel);


	/*Nodesystem*/
	ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.f);
	ECS::Instance()->getSystem<OctreeAddRemoverSystem>()->update(0.f);
	for ( size_t i = 0; i < size; i++ ) {
		conns.clear();
		x_cur = i % x_max;
		z_cur = static_cast<int>(floor(i / x_max));
		glm::vec3 pos(x_cur * padding - offsetX, offsetY, z_cur * padding - offsetZ);

		bool blocked = false;
		e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setPosition(pos);
		std::vector < Octree::CollisionInfo> vec;
		m_octree->getCollisions(e.get(), &vec);

		for ( Octree::CollisionInfo& info : vec ) {
			int j = ( info.entity->getName().compare("Map_") );
			if ( j >= 0 ) {
				//Not walkable

				blocked = true;
				break;
			}
		}

		nodes.emplace_back(pos, blocked, i);

		for ( int dx = -1; dx <= 1; dx++ ) {
			for ( int dz = -1; dz <= 1; dz++ ) {
				if ( dx == 0 && dz == 0 )
					continue;

				int nx = x_cur + dx;
				int nz = z_cur + dz;
				if ( nx >= 0 && nx < x_max && nz >= 0 && nz < z_max ) {
					int ni = nx + nz * x_max;
					conns.push_back(ni);
				}
			}
		}

		connections.push_back(conns);
	}
	//Delete "DeleteMeFirstFrameDummy"
	ECS::Instance()->destroyEntity(e);

	m_nodeSystem->setNodes(nodes, connections);
	Memory::SafeDeleteArr(walkable);
}


bool AiSystem::addEntity(Entity* entity) {
	bool returnValue = BaseComponentSystem::addEntity(entity);

	AiEntity aiEntity;
	aiEntity.transComp = entity->getComponent<TransformComponent>();
	aiEntity.physComp = entity->getComponent<PhysicsComponent>();
	aiEntity.aiComp = entity->getComponent<AiComponent>();

	m_aiEntities.try_emplace(entity->getID(), aiEntity);

	return returnValue;
}


std::vector<Entity*>& AiSystem::getEntities() {
	return entities;
}

void AiSystem::update(float dt) {
	std::vector<std::future<void>> futures;
	for ( auto& entity : entities ) {
		// Might be dangerous for threads
		futures.push_back(Application::getInstance()->pushJobToThreadPool([this, entity, dt] (int id) { this->aiUpdateFunc(entity, dt); }));
	}
	for ( auto& a : futures ) {
		a.get();
	}
}

void AiSystem::updatePath(AiComponent* aiComp, TransformComponent* transComp) {
	if ( aiComp->posTarget != aiComp->lastTargetPos ) {

		aiComp->lastTargetPos = aiComp->posTarget;

		auto tempPath = m_nodeSystem->getPath(transComp->getTranslation(), aiComp->posTarget);

		aiComp->currNodeIndex = 0;

		// Fix problem of always going toward closest node
		if ( tempPath.size() > 1 && glm::distance(tempPath[1].position, transComp->getTranslation()) < glm::distance(tempPath[1].position, tempPath[0].position) ) {
			aiComp->currNodeIndex += 1;
		}

		aiComp->currPath = tempPath;
	}
}

void AiSystem::entityTargetFunc(AiComponent* aiComp, TransformComponent* transComp, GunComponent* gunComp) {
	if ( aiComp->entityTarget != nullptr ) {
		if ( aiComp->timeTakenOnPath > m_timeBetweenPathUpdate ) {
			aiComp->timeTakenOnPath = 0.f;
			aiComp->posTarget = aiComp->entityTarget->getComponent<TransformComponent>()->getTranslation();
			aiComp->reachedTarget = false;

			updatePath(aiComp, transComp);
		}

		if ( gunComp != nullptr ) {
			// Approx gun pos
			auto gunPos = transComp->getTranslation() + glm::vec3(0.f, 0.9f, 0.f);
			// Approx enemy head pos
			auto enemyPos = aiComp->entityTarget->getComponent<TransformComponent>()->getTranslation() + glm::vec3(0.f, 0.9f, 0.f);
			auto fireDir = enemyPos - gunPos;
			fireDir = glm::normalize(fireDir);

			float hitDist = Intersection::rayWithAabb(gunPos, fireDir, *aiComp->entityTarget->getComponent<BoundingBoxComponent>()->getBoundingBox());

			Octree::RayIntersectionInfo rayHitInfo;
			m_octree->getRayIntersection(gunPos + fireDir /*In order to (hopefully) miss itself*/, fireDir, &rayHitInfo);
			if ( hitDist < 7.f && glm::abs(hitDist - glm::distance(enemyPos, gunPos)) < 1.f && hitDist < rayHitInfo.closestHit ) {
				gunComp->setFiring(gunPos += fireDir, fireDir);

				if ( fireDir.z < 0.f ) {
					transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) + 1.5707f, 0.f);
				} else {
					transComp->setRotations(0.f, glm::atan(fireDir.x / fireDir.z) - 1.5707f, 0.f);
				}
			}
		}
	}
}

void AiSystem::aiUpdateFunc(Entity* entity, const float dt) {
	auto aiComp = entity->getComponent<AiComponent>();
	auto transComp = entity->getComponent<TransformComponent>();
	auto physComp = entity->getComponent<PhysicsComponent>();
	auto gunComp = entity->getComponent<GunComponent>();

	for ( auto& e : entity->getChildEntities() ) {
		auto candle = e->getComponent<CandleComponent>();
		if ( candle ) {
			if ( !candle->getIsAlive() ) {
				return;
			}
		}
	}

	entityTargetFunc(aiComp, transComp, gunComp);

	if ( aiComp->currPath.empty() ) {
		return;
	}

	if ( !aiComp->reachedTarget ) {
		aiComp->timeTakenOnPath += dt;

		// Check if the distance between target and ai is low enough
		if ( glm::distance(transComp->getTranslation(), aiComp->currPath[aiComp->currNodeIndex].position) < aiComp->targetReachedThreshold ) {
			aiComp->lastVisitedNode = aiComp->currPath[aiComp->currNodeIndex];
			aiComp->reachedTarget = true;
		} else {
			glm::vec3 desiredDir = aiComp->currPath[aiComp->currNodeIndex].position - transComp->getTranslation();
			if ( desiredDir == glm::vec3(0.f) ) {
				desiredDir = glm::vec3(1.0f, 0.f, 0.f);
			}
			desiredDir = glm::normalize(desiredDir);
			glm::vec3 desiredVel = desiredDir * aiComp->movementSpeed;
			glm::vec3 steering = ( desiredVel - physComp->velocity );
			float steeringMag = ( glm::length(steering) * aiComp->mass );
			steering *= aiComp->maxSteeringForce / ( steeringMag != 0.f ? steeringMag : 1.f );
			physComp->velocity += steering;
			// Float used to clamp the magnitude of the velocity between 0 and m_movementSpeed
			float velMagClamper = glm::length(physComp->velocity);
			velMagClamper = ( velMagClamper > aiComp->movementSpeed ) ? aiComp->movementSpeed / velMagClamper : 1.0f;
			physComp->velocity = physComp->velocity * velMagClamper;

			if ( !gunComp->firing ) {
				auto dir = normalize(physComp->velocity);				
				if ( physComp->velocity.z < 0.f ) {
					transComp->setRotations(0.f, glm::atan(physComp->velocity.x / physComp->velocity.z) + 1.5707f, 0.f);
				} else {
					transComp->setRotations(0.f, glm::atan(physComp->velocity.x / physComp->velocity.z) - 1.5707f, 0.f);
				}
			}
		}
	} else if ( aiComp->currNodeIndex < aiComp->currPath.size() - 1 ) {
		aiComp->currNodeIndex++;
		aiComp->posTarget = aiComp->currPath[aiComp->currNodeIndex].position;
		aiComp->reachedTarget = false;
	} else {
		physComp->velocity = glm::vec3(0.f);
	}
}
