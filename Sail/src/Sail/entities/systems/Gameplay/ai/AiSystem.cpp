#include "pch.h"
#include "AiSystem.h"
#include "../../../components/AiComponent.h"
#include "../../../components/PhysicsComponent.h"
#include "../../../components/TransformComponent.h"
#include "../../../components/CandleComponent.h"
#include "../../../components/GunComponent.h"
#include "../../../components/FSMComponent.h"
#include "Sail/ai/pathfinding/NodeSystem.h"

#include "../../../ECS.h"
#include "../../../components/BoundingBoxComponent.h"
#include "../../physics/UpdateBoundingBoxSystem.h"
#include "../../physics/OctreeAddRemoverSystem.h"
#include "Sail/utils/Utils.h"
#include "../../Physics/Octree.h"
#include "Sail/Application.h"
#include "../../Physics/Intersection.h"
#include "../../Physics/Physics.h"

AiSystem::AiSystem() {
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<PhysicsComponent>(true, true, true);
	registerComponent<AiComponent>(true, true, true);
	registerComponent<FSMComponent>(true, true, true);
	registerComponent<GunComponent>(true, true, true);
	registerComponent<CandleComponent>(false, true, false);

	m_nodeSystem = std::make_unique<NodeSystem>();
	m_timeBetweenPathUpdate = 0.5f;
}

AiSystem::~AiSystem() {}

#ifdef _DEBUG_NODESYSTEM
void AiSystem::initNodeSystem(Model* bbModel, Octree* octree, Shader* shader) {
	m_nodeSystem->setDebugModelAndScene(shader);
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
	e->addComponent<BoundingBoxComponent>(bbModel)->getBoundingBox()->setHalfSize(glm::vec3(0.7f, .9f, 0.7f));


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

NodeSystem* AiSystem::getNodeSystem() {
	return m_nodeSystem.get();
}


void AiSystem::aiUpdateFunc(Entity* entity, const float dt) {
	entity->getComponent<FSMComponent>()->update(dt, entity);
	auto transComp = entity->getComponent<TransformComponent>();
	auto aiComp = entity->getComponent<AiComponent>();

	if ( aiComp->timeTakenOnPath > m_timeBetweenPathUpdate && aiComp->doWalk ) {
		aiComp->updatePath = true;
	}

	updatePath(aiComp, transComp);
	updatePhysics(aiComp, transComp, entity->getComponent<PhysicsComponent>(), dt);
}

glm::vec3& AiSystem::getDesiredDir(AiComponent* aiComp, TransformComponent* transComp) {
	glm::vec3 desiredDir = aiComp->currPath[aiComp->currNodeIndex].position - transComp->getTranslation();
	if ( desiredDir == glm::vec3(0.f) ) {
		desiredDir = glm::vec3(1.0f, 0.f, 0.f);
	}
	desiredDir = glm::normalize(desiredDir);
	return desiredDir;
}


void AiSystem::updatePath(AiComponent* aiComp, TransformComponent* transComp) {
	if ( aiComp->updatePath ) {

		aiComp->timeTakenOnPath = 0.f;
		aiComp->reachedPathingTarget = false;

		// aiComp->posTarget is updated in each FSM state
		aiComp->lastTargetPos = aiComp->posTarget;

		auto tempPath = m_nodeSystem->getPath(transComp->getTranslation(), aiComp->posTarget);

		aiComp->currNodeIndex = 0;

		// Fix problem of always going toward closest node
		if ( tempPath.size() > 1 && glm::distance(tempPath[1].position, transComp->getTranslation()) < glm::distance(tempPath[1].position, tempPath[0].position) ) {
			aiComp->currNodeIndex += 1;
		}

		aiComp->currPath = tempPath;

		aiComp->updatePath = false;
	}
}

void AiSystem::updatePhysics(AiComponent* aiComp, TransformComponent* transComp, PhysicsComponent* physComp, float dt) {
	// Check if there is a path currently active and if the ai should be walking
	if ( aiComp->currPath.size() > 0 && aiComp->doWalk ) {
		// Check if the ai hasn't reached the current pathing target yet
		if ( !aiComp->reachedPathingTarget ) {
			aiComp->timeTakenOnPath += dt;

			// Check if the distance between current node target and ai is low enough to begin targeting next node
			if ( glm::distance(transComp->getTranslation(), aiComp->currPath[aiComp->currNodeIndex].position) < aiComp->targetReachedThreshold ) {
				aiComp->lastVisitedNode = aiComp->currPath[aiComp->currNodeIndex];
				aiComp->reachedPathingTarget = true;
			// Else continue walking
			} else {
				float acceleration = 70.0f - ( glm::length(physComp->velocity) / physComp->maxSpeed ) * 20.0f;
				physComp->accelerationToAdd = getDesiredDir(aiComp, transComp) * acceleration;
			}
		// Else increment the current node path
		} else if ( aiComp->currPath.size() > 0 ) {
			// Update next node target
			if ( aiComp->currNodeIndex < aiComp->currPath.size() - 1 ) {
				aiComp->currNodeIndex++;
			}
			aiComp->reachedPathingTarget = false;
		}

		float newYaw = getAiYaw(physComp, transComp->getRotations().y, dt);
		transComp->setRotations(0.f, newYaw, 0.f);

	// If the ai shouldn't walk, just stop walking
	} else {
		// Set velocity to 0
		physComp->velocity = glm::vec3(0.f, physComp->velocity.y, 0.f);
		aiComp->timeTakenOnPath = 3.f;
	}
}

float AiSystem::getAiYaw(PhysicsComponent* physComp, float currYaw, float dt) {
	float newYaw = currYaw;
	if ( glm::length2(physComp->velocity) > 0.f ) {
		float desiredYaw = 0.f;
		// TODO: Edit turnrate and things
		float turnRate = PI_2; // 2 pi
		auto normalizedVel = glm::normalize(physComp->velocity);
		float physCompX = normalizedVel.x;
		float physCompZ = normalizedVel.z;
		physCompZ = physCompZ != 0 ? physCompZ : 0.1f;
		if ( physComp->velocity.z < 0.f ) {
			desiredYaw = glm::atan(physCompX / physCompZ) + 1.5707f;
		} else {
			desiredYaw = glm::atan(physCompX / physCompZ) - 1.5707f;
		}
		desiredYaw = Utils::wrapValue(desiredYaw, 0.f, PI_2);
		float diff = desiredYaw - currYaw;

		if ( std::abs(diff) > PI ) {
			diff = currYaw - desiredYaw;
		}

		float toTurn = 0.f;
		if ( diff > 0 ) {
			toTurn = turnRate * dt;
		} else if ( diff < 0 ) {
			toTurn = -turnRate * dt;
		}

		if ( std::abs(diff) > std::abs(toTurn) ) {
			newYaw = Utils::wrapValue(currYaw + toTurn, 0.f, PI_2);
		} else {
			newYaw = Utils::wrapValue(currYaw + diff, 0.f, PI_2);
		}

	}
	return newYaw;
}
