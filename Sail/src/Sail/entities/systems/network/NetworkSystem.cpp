#include "pch.h"
#include "NetworkSystem.h"
#include "Network/NWrapperSingleton.h"
#include "../../components/Component.h"

NetworkSystem::NetworkSystem() {
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
	requiredComponentTypes.push_back(OtherPlayerComponent::ID);
	readBits |= OtherPlayerComponent::BID;
	writeBits |= OtherPlayerComponent::BID;
	requiredComponentTypes.push_back(TransformComponent::ID);
	readBits |= TransformComponent::BID;
	writeBits |= TransformComponent::BID;
}

void NetworkSystem::initialize(Entity* playerEntity) {
	m_playerEntity = playerEntity;
}

void NetworkSystem::initalizeStructPackages() {
	m_arr_structs = new StructPackage*[STRUCTTYPES::COUNT];
	m_arr_structs[STRUCTTYPES::TRANSFORM] = new TransformStruct;
}

void NetworkSystem::parsePackage(std::string& deSerializedData) {
	// The entire package, structured as follows:
	/*
		- EntityCount
			- ID
			- TYPE
			- DATA
				- ID
				- TYPE
				- DATA
			- ID
			- TYPE
			- DATA
				- ID
				- TYPE
				- DATA
			- ID
			- TYPE
			- DATA
			.
			.
			.
			n = EntityCount
	*/
	// Parse the EntityCount and loop based on it
	char EntityCount = deSerializedData.at(0);
	deSerializedData.erase(0, 1);	// Erase it (offset 0, elementsToErase 1)

	// Loop per entity
	for (size_t i = 0; i < EntityCount; i++) {
		// Parse the ID
		char entityId = deSerializedData.at(0);
		deSerializedData.erase(0, 1);

		// Parse the struct TYPE
		char structType = deSerializedData.at(0);
		deSerializedData.erase(0, 1);

		// Parse the data depending on the TYPE
		m_arr_structs[structType]->parse(deSerializedData);
	}
}
