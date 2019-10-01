#include "pch.h"
#include "NetworkClientSystem.h"
#include "../../components/Component.h"
#include "Sail/entities/components/TransformComponent.h"
#include "../../Entity.h"
#include "../../SPLASH/src/game/events/NetworkSerializedPackageEvent.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/types/vector.hpp"

NetworkClientSystem::NetworkClientSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<TransformComponent>(true, true, true);
}

NetworkClientSystem::~NetworkClientSystem() {

}

void NetworkClientSystem::update(float dt) {
	m_network->checkForPackages();

	sendPlayerTranslationToHost();
}

bool NetworkClientSystem::onSerializedPackageRecieved(NetworkSerializedPackageEvent& event) {
	// Fetch data from networkWrapper and de-serialize it
	std::istringstream is(event.getSerializedData());
	cereal::PortableBinaryInputArchive inputArchive(is);
	myStruct deSerializedData;
	inputArchive(deSerializedData);

	for (auto& e : entities) {
		// Fetch the transformComponent of the entity which represents the other player
		TransformComponent* transform = e->getComponent<TransformComponent>();

		// Alter the transform of that entity based on the recieved movement of the other player
		transform->setTranslation(glm::vec3{
			deSerializedData.trans.x,
			deSerializedData.trans.y,
			deSerializedData.trans.z,
			});
	}


	return false;
}

void NetworkClientSystem::sendPlayerTranslationToHost() {
	TransformComponent* transform = m_playerEntity->getComponent<TransformComponent>();
	glm::vec3 translation = transform->getTranslation();

	myStruct data;

	//TranslationStruct data;
	data.trans.x = translation.x;
	data.trans.y = translation.y;
	data.trans.z = translation.z;

	std::ostringstream os(std::ios::binary);
	cereal::PortableBinaryOutputArchive outputArchive(os);
	outputArchive(data);
	std::string binaryData = os.str();

	m_network->sendSerializedDataToHost(binaryData);
}
