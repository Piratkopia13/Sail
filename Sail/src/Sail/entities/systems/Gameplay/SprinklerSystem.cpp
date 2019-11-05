#include "pch.h"

#include "SprinklerSystem.h"

#include "Sail/entities/components/MapComponent.h"
#include "Sail/entities/ECS.h"


SprinklerSystem::SprinklerSystem() : BaseComponentSystem() {
	registerComponent<MapComponent>(true, true, false);
}

SprinklerSystem::~SprinklerSystem() {

}

void SprinklerSystem::update(float dt) {
	for (auto& e : entities) {
		MapComponent* map = e->getComponent<MapComponent>();

		endGameTimer += dt;
		if (endGameTimer > endGameStartLimit) {
			
			if (active) {
				for (int x = 0 + endGameMapIncrement; x < MapComponent::xsize - endGameMapIncrement; x++) {
					addToActiveRooms(map->tileArr[x][endGameMapIncrement][1]);
					addToActiveRooms(map->tileArr[x][MapComponent::ysize - endGameMapIncrement][1]);
				}
				for (int y = 1 + endGameMapIncrement; y < MapComponent::ysize - endGameMapIncrement - 1; y++) {
					addToActiveRooms(map->tileArr[endGameMapIncrement][y][1]);
					addToActiveRooms(map->tileArr[MapComponent::xsize - endGameMapIncrement][y][1]);
				}
				active = false;
				endGameMapIncrement++;
				

				Logger::Log("Number of active rooms in sprinker system: " + std::to_string(activeRooms.size()));
				Logger::Log("Current Rooms active in sprinkler system:");
				for (auto rooms : activeRooms) {
					Logger::Log(std::to_string(rooms));
				}
			}
			else if (((endGameTimer - endGameStartLimit) / endGameTimeIncrement) > static_cast<float>(endGameMapIncrement)) {
				active = true;
			}

			

		}

		
	}

}

void SprinklerSystem::addToActiveRooms(int room) {
	
	if (room != 0) {
		std::vector<int>::iterator it = std::find(activeRooms.begin(), activeRooms.end(), room);
		if (it == activeRooms.end()) {
			activeRooms.push_back(room);
		}
	}
	if (room > 100) {
		int bajs = 0;
	}
}