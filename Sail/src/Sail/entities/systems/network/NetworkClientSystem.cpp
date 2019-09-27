#include "pch.h"
#include "NetworkClientSystem.h"
#include "../../components/Component.h"
#include "Sail/entities/components/TransformComponent.h"
#include "../../Entity.h"
#include "../../SPLASH/src/game/events/NetworkSerializedPackageEvent.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/types/vector.hpp"
#include <iostream> // TODO: remove

NetworkClientSystem::NetworkClientSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
}

NetworkClientSystem::~NetworkClientSystem() {

}

void NetworkClientSystem::update(float dt) {
	m_network->checkForPackages();

	sendPlayerTranslationToHost();
}

bool NetworkClientSystem::onSerializedPackageRecieved(NetworkSerializedPackageEvent& event) {
	std::istringstream is(event.getSerializedData());
	cereal::PortableBinaryInputArchive inputArchive(is);
	TranslationStruct backToNormal;
	inputArchive(backToNormal);	// After this line the trans data is stored in backToNormal
	std::cout << "Recieved translation package from host:";
	std::cout << "x" << backToNormal.trans.x;
	std::cout << "y" << backToNormal.trans.y;
	std::cout << "z" << backToNormal.trans.z << "\n";
	return false;
}

void NetworkClientSystem::sendPlayerTranslationToHost() {
	TransformComponent* transform = m_playerEntity->getComponent<TransformComponent>();
	glm::vec3 translation = transform->getTranslation();

	translation.r;

	TranslationStruct data;
	data.trans.x = translation.x;
	data.trans.y = translation.y;
	data.trans.z = translation.z;

	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive outputArchive(os);
	outputArchive(data);
	std::string binaryData = os.str();

	m_network->sendSerializedData(binaryData);
}
