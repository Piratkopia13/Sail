#include "pch.h"

#include "SprinklerSystem.h"

#include "Sail/entities/components/MapComponent.h"
#include "Sail/entities/components/SpotlightComponent.h"
#include "Sail/entities/ECS.h"
#include "Sail/utils/Utils.h"

SprinklerSystem::SprinklerSystem() : BaseComponentSystem() {
	registerComponent<MapComponent>(true, true, false);
}

SprinklerSystem::~SprinklerSystem() {

}

void SprinklerSystem::update(float dt) {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();
		endGameTimer += dt;
		// End game is reached, sprinklers starting
		if (endGameTimer > endGameStartLimit) {

			if (activateSprinklers) {
				// Pick a random side of the map, and activates all rooms on that side
				int mapSide = static_cast<int>(Utils::rnd() * 3.f + 1.f);
				Logger::Log("Side: " + std::to_string(mapSide));
				switch (mapSide) {
				case 1:
					for (int x = 0 + xMinIncrement; x < MapComponent::xsize - xMaxIncrement; x++) {
						addToActiveRooms(map->tileArr[x][yMinIncrement][1]);
					}
					yMinIncrement++;
					break;
				case 2:
					for (int x = 0 + xMinIncrement; x < MapComponent::xsize - xMaxIncrement; x++) {
						addToActiveRooms(map->tileArr[x][MapComponent::ysize - 1 - yMaxIncrement][1]);
					}
					yMaxIncrement++;
					break;
				case 3:
					for (int y = 0 + yMinIncrement; y < MapComponent::ysize - yMaxIncrement; y++) {
						addToActiveRooms(map->tileArr[xMinIncrement][y][1]);
					}
					xMinIncrement++;
					break;
				case 4:
					for (int y = 0 + yMinIncrement; y < MapComponent::ysize - yMaxIncrement; y++) {
						addToActiveRooms(map->tileArr[MapComponent::xsize - 1 - xMaxIncrement][y][1]);
					}
					xMaxIncrement++;
					break;
				default:
					break;
				}
				activateSprinklers = false;
				endGameMapIncrement++;
			}
			// New time increment reached, add new rooms
			else if (((endGameTimer - endGameStartLimit) / endGameTimeIncrement) > static_cast<float>(endGameMapIncrement)) {
				activateSprinklers = true;
			}



		}

	}
}

std::vector<int> SprinklerSystem::getActiveRooms() const
{
	return activeRooms;
}

void SprinklerSystem::addToActiveRooms(int room) {
	if (room != 0) {
		std::vector<int>::iterator it = std::find(activeRooms.begin(), activeRooms.end(), room);
		if (it == activeRooms.end()) {
			activeRooms.push_back(room);
		}
	}
}

