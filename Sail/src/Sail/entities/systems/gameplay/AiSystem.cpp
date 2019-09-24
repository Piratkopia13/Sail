#include "pch.h"
#include "AiSystem.h"
#include "../../components/AiComponent.h"
#include "../../components/PhysicsComponent.h"
#include "../../components/TransformComponent.h"
#include "Sail/ai/pathfinding/NodeSystem.h"

#include "../../ECS.h"
#include "../../components/BoundingBoxComponent.h"
#include "../physics/UpdateBoundingBoxSystem.h"
#include "../physics/OctreeAddRemoverSystem.h"
#include "Sail/utils/Utils.h"
#include "../Physics/Octree.h"

AiSystem::AiSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
	requiredComponentTypes.push_back(PhysicsComponent::ID);
	requiredComponentTypes.push_back(AiComponent::ID);
}

AiSystem::~AiSystem() {}

void AiSystem::initNodeSystem(Model* bbModel, Octree* octree) {
	/* "Unit test" for NodeSystem */
	m_nodeSystem = std::make_unique<NodeSystem>();

#ifdef _DEBUG_NODESYSTEM
	Model* nodeSystemModel = &m_app->getResourceManager().getModel("sphere.fbx", shader);
	nodeSystemModel->getMesh(0)->getMaterial()->setDiffuseTexture("missing.tga");
	test->setDebugModelAndScene(nodeSystemModel, &m_scene);
#endif

	std::vector<NodeSystem::Node> nodes;
	std::vector<std::vector<unsigned int>> connections;

	std::vector<unsigned int> conns;
	int x_max = 60;
	int z_max = 60;
	int x_cur = 0;
	int z_cur = 0;
	int size = x_max * z_max;

	int padding = 2;
	float offsetX = x_max * padding * 0.5f;
	float offsetZ = z_max * padding * 0.5f;
	float offsetY = 0;
	bool* walkable = SAIL_NEW bool[size];

	auto e = ECS::Instance()->createEntity("DeleteMeFirstFrameDummy");
	//e->addComponent<TransformComponent>(glm::vec3(0.f, 0.f, 0.f));
	//e->addComponent<ModelComponent>(m_boundingBoxModel.get());
	e->addComponent<BoundingBoxComponent>(bbModel);
	//m_scene.addEntity(e);


	/*Nodesystem*/
	//ECS::Instance()->update(0.0f); // Update Boundingboxes/octree system here
	ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.f);
	ECS::Instance()->getSystem<OctreeAddRemoverSystem>()->update(0.f);
	for ( size_t i = 0; i < size; i++ ) {
		conns.clear();
		x_cur = i % x_max;
		z_cur = floor(i / x_max);
		glm::vec3 pos(x_cur * padding - offsetX, offsetY, z_cur * padding - offsetZ);

		bool blocked = false;
		e->getComponent<BoundingBoxComponent>()->getBoundingBox()->setPosition(pos);
		std::vector < Octree::CollisionInfo> vec;
		octree->getCollisions(e.get(), &vec);

		for ( Octree::CollisionInfo& info : vec ) {
			int i = ( info.entity->getName().compare("Map_") );
			if ( i >= 0 ) {
				//Not walkable
				//auto e2 = ECS::Instance()->createEntity("blockedGroundMarker");
				//e2->addComponent<TransformComponent>(pos);
				//e2->addComponent<ModelComponent>(m_boundingBoxModel.get());
				//m_scene.addEntity(e2);

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


void AiSystem::addEntity(Entity* entity) {
	BaseComponentSystem::addEntity(entity);

	AiEntity aiEntity;
	aiEntity.transComp = entity->getComponent<TransformComponent>();
	aiEntity.physComp = entity->getComponent<PhysicsComponent>();
	aiEntity.aiComp = entity->getComponent<AiComponent>();

	m_aiEntities.try_emplace(entity->getID(), aiEntity);
}

void AiSystem::removeEntity(Entity* entity) {
	BaseComponentSystem::removeEntity(entity);

}

std::vector<Entity*>& AiSystem::getEntities() {
	return entities;
}

void AiSystem::update(float dt) {
	for ( auto& entity : m_aiEntities ) {

		if ( entity.second.aiComp->entityTarget != nullptr ) {
			entity.second.aiComp->timeTakenOnPath += dt;
			if ( entity.second.aiComp->timeTakenOnPath > m_timeBetweenPathUpdate ) {
				entity.second.aiComp->timeTakenOnPath = 0.f;
				entity.second.aiComp->posTarget = entity.second.aiComp->entityTarget->getComponent<TransformComponent>()->getTranslation();
				entity.second.aiComp->reachedTarget = false;

				updatePath(entity.second.aiComp, entity.second.transComp);
			}
		}

		if ( entity.second.aiComp->currPath.empty() ) {
			return;
		}

		if ( entity.second.aiComp->reachedTarget && entity.second.aiComp->currNodeIndex < entity.second.aiComp->currPath.size() - 1 ) {
			entity.second.aiComp->currNodeIndex++;
			entity.second.aiComp->posTarget = entity.second.aiComp->currPath[entity.second.aiComp->currNodeIndex].position;
			entity.second.aiComp->reachedTarget = false;
		}

		if ( !entity.second.aiComp->reachedTarget ) {
			// Check if the distance between target and ai is low enough
			if ( glm::distance(entity.second.transComp->getTranslation(), entity.second.aiComp->currPath[entity.second.aiComp->currNodeIndex].position) < entity.second.aiComp->targetReachedThreshold ) {
				entity.second.aiComp->lastVisitedNode = entity.second.aiComp->currPath[entity.second.aiComp->currNodeIndex];
				entity.second.aiComp->reachedTarget = true;
			} else {
				glm::vec3 desiredDir = entity.second.aiComp->currPath[entity.second.aiComp->currNodeIndex].position - entity.second.transComp->getTranslation();
				if ( desiredDir == glm::vec3(0.f) ) {
					desiredDir = glm::vec3(1.0f, 0.f, 0.f);
				}
				desiredDir = glm::normalize(desiredDir);
				glm::vec3 desiredVel = desiredDir * entity.second.aiComp->movementSpeed;
				glm::vec3 steering = ( desiredVel - entity.second.physComp->velocity );
				float steeringMag = ( glm::length(steering) * entity.second.aiComp->mass );
				steering *= entity.second.aiComp->maxSteeringForce / ( steeringMag != 0.f ? steeringMag : 1.f );
				entity.second.physComp->velocity += steering;
				// Float used to clamp the magnitude of the velocity between 0 and m_movementSpeed
				float velMagClamper = glm::length(entity.second.physComp->velocity);
				velMagClamper = ( velMagClamper > entity.second.aiComp->movementSpeed ) ? entity.second.aiComp->movementSpeed / velMagClamper : 1.0f;
				entity.second.physComp->velocity = entity.second.physComp->velocity * velMagClamper;
				//Logger::Log("Velocity: " + Utils::toStr(m_physComp->velocity));
			}
		} else {
			entity.second.physComp->velocity = glm::vec3(0.f);
		}
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
