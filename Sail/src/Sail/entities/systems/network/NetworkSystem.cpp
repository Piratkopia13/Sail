#include "pch.h"
#include "NetworkSystem.h"
#include "../../components/Component.h"
#include "Sail/entities/components/TransformComponent.h"
#include "../../Entity.h"

NetworkClientSystem::NetworkClientSystem() {
	requiredComponentTypes.push_back(TransformComponent::ID);
}

NetworkClientSystem::~NetworkClientSystem() {

}

void NetworkClientSystem::update(float dt) {
	// Go through the list of non-deterministic components and send data to host
	
	std::vector<TransformSnapshot> package;
	for (auto& entity : entities) {
		TransformComponent* transform = entity->getComponent<TransformComponent>();
		TransformSnapshot transSnap = transform->getCurrentTransformState();
		package.push_back(transSnap);
	}
}
