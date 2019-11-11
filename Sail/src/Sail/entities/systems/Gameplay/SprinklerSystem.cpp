#include "pch.h"

#include "SprinklerSystem.h"


#include "Sail/entities/components/SpotlightComponent.h"
#include "Sail/entities/ECS.h"
#include "Sail/utils/Utils.h"
#include "Sail.h"
#include "Sail/entities/systems/Gameplay/LevelSystem/LevelSystem.h"
#include "Network/NWrapperSingleton.h"

SprinklerSystem::SprinklerSystem() : BaseComponentSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
	m_map = ECS::Instance()->getSystem<LevelSystem>();
	m_settings = &Application::getInstance()->getSettings();
	m_endGameStartLimit = m_settings->gameSettingsDynamic["map"]["sprinklerTime"].value;
	m_endGameTimeIncrement = m_settings->gameSettingsDynamic["map"]["sprinklerIncrement"].value;
}

SprinklerSystem::~SprinklerSystem() {

}

void SprinklerSystem::stop() {
	m_enableNewSprinklers = false;
	m_endGameTimer = 0.f;
	m_endGameMapIncrement = 0;
	m_xMinIncrement = 0;
	m_xMaxIncrement = 0;
	m_yMinIncrement = 0;
	m_yMaxIncrement = 0;
	m_mapSide = 0;
	m_activeRooms.clear();
}

void SprinklerSystem::update(float dt) {

	for (auto& e : entities) {
		// End game is reached, sprinklers starting
		if (!m_enableSprinklers && NWrapperSingleton::getInstance().isHost()) {
			m_endGameTimer += dt;
			if (m_endGameTimer > m_endGameStartLimit) {
				m_enableSprinklers = true;
				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
					Netcode::MessageType::ENABLE_SPRINKLERS,
					SAIL_NEW Netcode::MessageEnableSprinklers{}
				);
			}
		}
		if (m_enableSprinklers) {
			m_endGameTimer += dt;

			CandleComponent* candle = e->getComponent<CandleComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();

			float candlePosX;
			float candlePosZ;

			if (candle->isCarried && candle->wasCarriedLastUpdate && !e->isAboutToBeDestroyed()) {
				candlePosX = transform->getParent()->getTranslation().x;
				candlePosZ = transform->getParent()->getTranslation().z;
			}
			else {
				candlePosX = transform->getTranslation().x;
				candlePosZ = transform->getTranslation().z;
			}

			int candleLocationRoomID = m_map->getRoomIDWorldPos(candlePosX, candlePosZ);
			std::vector<int>::iterator it = std::find(m_activeRooms.begin(), m_activeRooms.end(), candleLocationRoomID);
			if (it != m_activeRooms.end()) {
				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
					Netcode::MessageType::HIT_BY_SPRINKLER,
					SAIL_NEW Netcode::MessageHitBySprinkler{
						e->getParent()->getComponent<NetworkReceiverComponent>()->m_id
					}
				);
			}

			// Add more active rooms
			if (m_enableNewSprinklers) {
				// Rotate between sides to activate
				m_mapSide++;
				m_mapSide = m_mapSide > 4 ? 1 : m_mapSide;
				switch (m_mapSide) {
				case 1:
					for (int x = 0 + m_xMinIncrement; x < m_map->xsize - m_xMaxIncrement; x++) {
						addToActiveRooms(m_map->getRoomID(x, m_yMinIncrement));
					}
					m_yMinIncrement++;
					break;
				case 2:
					for (int x = 0 + m_xMinIncrement; x < m_map->xsize - m_xMaxIncrement; x++) {
						addToActiveRooms(m_map->getRoomID(x, m_map->ysize - 1 - m_yMaxIncrement));
					}
					m_yMaxIncrement++;
					break;
				case 3:
					for (int y = 0 + m_yMinIncrement; y < m_map->ysize - m_yMaxIncrement; y++) {
						addToActiveRooms(m_map->getRoomID(m_xMinIncrement, y));
					}
					m_xMinIncrement++;
					break;
				case 4:
					for (int y = 0 + m_yMinIncrement; y < m_map->ysize - m_yMaxIncrement; y++) {
						addToActiveRooms(m_map->getRoomID(m_map->xsize - 1 - m_xMaxIncrement, y));
					}
					m_xMaxIncrement++;
					break;
				default:
					break;
				}
				m_enableNewSprinklers = false;
				m_endGameMapIncrement++;
			}
			// New time increment reached, add new rooms next update
			else if (((m_endGameTimer) / m_endGameTimeIncrement) > static_cast<float>(m_endGameMapIncrement)) {
				m_enableNewSprinklers = true;
			}



		}
	}
}

const std::vector<int>& SprinklerSystem::getActiveRooms() const
{
	return m_activeRooms;
}

void SprinklerSystem::addToActiveRooms(int room) {
	if (room != 0) {
		std::vector<int>::iterator itRooms = std::find(m_activeRooms.begin(), m_activeRooms.end(), room);
		if (itRooms == m_activeRooms.end()) {
			m_activeRooms.push_back(room);
		}
	}
}

void SprinklerSystem::enableSprinklers() {
	m_enableSprinklers = true;
	m_endGameTimer = 0.f;
}