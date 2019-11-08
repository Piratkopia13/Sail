#include "pch.h"

#include "SprinklerSystem.h"


#include "Sail/entities/components/SpotlightComponent.h"
#include "Sail/entities/ECS.h"
#include "Sail/utils/Utils.h"
#include "Sail.h"
#include "Sail/entities/systems/Gameplay/LevelSystem/LevelSystem.h"

SprinklerSystem::SprinklerSystem() : BaseComponentSystem() {
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
	m_endGameTimer += dt;
	// End game is reached, sprinklers starting
	if (m_endGameTimer > m_endGameStartLimit) {

		if (m_enableNewSprinklers) {
			// Add rooms to different vector to enable sprinklers and turn off hazard lights
			m_activeSprinklers.insert(m_activeSprinklers.end(), m_activeRooms.begin(), m_activeRooms.end());
			m_activeRooms.clear();
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
		// New time increment reached, add new rooms
		else if (((m_endGameTimer - m_endGameStartLimit) / m_endGameTimeIncrement) > static_cast<float>(m_endGameMapIncrement)) {
			m_enableNewSprinklers = true;
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
		std::vector<int>::iterator itSprinklers = std::find(m_activeSprinklers.begin(), m_activeSprinklers.end(), room);
		if (itRooms == m_activeRooms.end() && itSprinklers == m_activeSprinklers.end()) {
			m_activeRooms.push_back(room);
		}
	}
}

