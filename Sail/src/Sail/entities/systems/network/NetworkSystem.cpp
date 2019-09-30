#include "pch.h"
#include "NetworkSystem.h"
#include "Network/NWrapperSingleton.h"
#include "../../components/Component.h"

NetworkSystem::NetworkSystem() {
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
	initReadWriteBits();
	initEntityArr();


}

NetworkSystem::~NetworkSystem() {
	
	//delete[] m_arr_structs;
	//delete m_arr_pEntities	// Only delete the array, not the pointers inside it!!!
}

void NetworkSystem::initialize(Entity* playerEntity) {
	m_playerEntity = playerEntity;
}

void NetworkSystem::initStructPackages() {
	//m_arr_structs = new StructPackage*[STRUCTTYPES::COUNT];
	//m_arr_structs[STRUCTTYPES::TRANSFORM] = new TransformStruct;
}

void NetworkSystem::initReadWriteBits() {
	requiredComponentTypes.push_back(OtherPlayerComponent::ID);
	readBits |= OtherPlayerComponent::BID;
	writeBits |= OtherPlayerComponent::BID;
	requiredComponentTypes.push_back(TransformComponent::ID);
	readBits |= TransformComponent::BID;
	writeBits |= TransformComponent::BID;
}

void NetworkSystem::initEntityArr() {
	// This function might need to be called from somewhere else rather than the constructor,
	// since it needs the entities to already have been created before initializing like this.
		// - If it is moved, make a big initialize and call all the other init functions there as well.

	//m_arr_pEntities = new Entity*[entities.size()];
	//int idCounter = 0;

	//// Per entity
	//for (auto& entity : entities) {
	//	// Insert to array
	//	m_arr_pEntities = &entity;

	//	// Insert to map
	//	m_entityIDMap.insert(entity->getID(), idCounter++); // Increment the IDCounter
	//}
}

void NetworkSystem::parsePackage(std::string& deSerializedData) {
	// The entire package, structured as follows:
	///*
	//	- EntityCount
	//		- ID
	//		- TYPE
	//		- DATA
	//			- ID
	//			- TYPE
	//			- DATA
	//		- ID
	//		- TYPE
	//		- DATA
	//			- ID
	//			- TYPE
	//			- DATA
	//		- ID
	//		- TYPE
	//		- DATA
	//		.
	//		.
	//		.
	//		n = EntityCount
	//*/
	// Parse the EntityCount and loop based on it
	//char EntityCount = deSerializedData.at(0);
	//deSerializedData.erase(0, 1);	// Erase it (offset 0, elementsToErase 1)

	//// Loop per entity
	//for (size_t i = 0; i < EntityCount; i++) {
	//	// Parse the ID
	//	char entityId = deSerializedData.at(0);
	//	deSerializedData.erase(0, 1);

	//	// Parse the struct TYPE
	//	char structType = deSerializedData.at(0);
	//	deSerializedData.erase(0, 1);

	//	// Parse the data depending on the TYPE
	//	Entity* targetEntity = m_arr_pEntities[m_entityIDMap.at(entityId)];
	//	switch (structType) {
	//		case STRUCTTYPES::TRANSFORM:{
	//			data = this->parseTransform();

	//			TransformComponent* pTransform = targetEntity->getComponent<TransformComponent>();
	//			pTransform->setTranslation(data);

	//			break;
	//		}

	//		case STRUCTTYPES::ROTATION: {
	//			this->parseRotation();

	//			break;
	//		}

	//		case STRUCTTYPES::TRANSFORM_ROTATION: {
	//			this->parseTransformRotation();

	//			break;
	//		}

	//		default: {
	//			break;
	//		}
	//	}


	//	
	//	Entity* targetEntity = m_arr_pEntities[m_entityIDMap.at(entityId)];

	//	// Input the parsed data to the entity
	//	m_arr_pEntities[m_entityIDMap.at(entityId)]->getComponent<NetworkComponent>()->insert(data);
	//	TransformComponent* pComponent = m_arr_pEntities[m_entityIDMap.at(entityId)]->getComponent<TransformComponent>();
	//	pComponent->setTranslation(data);
	//}
}