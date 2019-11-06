#include "pch.h"
#include "PlayerSystem.h"
#include "Sail/events/EventDispatcher.h"
#include "Sail/events/types/PlayerDiedEvent.h"
#include "Sail/../Network/NWrapperSingleton.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/NoEntityComponent.h"
#include "Sail/entities/Entity.h"

PlayerSystem::PlayerSystem() {
	registerComponent<NoEntityComponent>(true, true, true);
	EventDispatcher::Instance().subscribe(Event::Type::PLAYER_DEATH, this);
}

PlayerSystem::~PlayerSystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::PLAYER_DEATH, this);
}

bool PlayerSystem::onEvent(const Event& event) {
	auto onPlayerDied = [](const PlayerDiedEvent& e) {
		const auto& myPlayerID = NWrapperSingleton::getInstance().getMyPlayerID();

		// Remove candle entity
		e.killed->removeDeleteAllChildren();
		
		// Check if the player was the one who died
		if (Netcode::getComponentOwner(e.netIDofKilled) == myPlayerID) {
			// If my player died, I become a spectator
			e.killed->addComponent<SpectatorComponent>();
			e.killed->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.f);
			e.killed->getComponent<MovementComponent>()->velocity = glm::vec3(0.f);
			e.killed->getComponent<NetworkSenderComponent>()->removeAllMessageTypes();
			e.killed->removeComponent<GunComponent>();
			e.killed->removeComponent<AnimationComponent>();
			e.killed->removeComponent<ModelComponent>();

			// Move entity above the level and make it look down
			const auto& transform = e.killed->getComponent<TransformComponent>();
			auto pos = glm::vec3(transform->getCurrentTransformState().m_translation);
			pos.y = 20.f;
			transform->setStartTranslation(pos);
			const auto& middleOfLevel = glm::vec3(MapComponent::tileSize * MapComponent::xsize / 2.f, 0.f, MapComponent::tileSize * MapComponent::ysize / 2.f);
			const auto& dir = glm::normalize(middleOfLevel - pos);
			const auto& rots = Utils::getRotations(dir);
			transform->setRotations(glm::vec3(0.f, -rots.y, rots.x));
		} else {
			e.killed->queueDestruction();
		}
	};


	switch (event.type) {
	case Event::Type::PLAYER_DEATH: onPlayerDied((const PlayerDiedEvent&)event); break;
	default: break;
	}

	return true;
}
