#include "pch.h"
#include "WaterCleaningSystem.h"
#include "Sail.h"

#include <iomanip>

WaterCleaningSystem::WaterCleaningSystem()
	: m_rendererWrapperRef(Application::getInstance()->getRenderWrapper()) 
{
	registerComponent<WaterCleaningComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);

	m_powerUpThreshold = 0.5f;
}

WaterCleaningSystem::~WaterCleaningSystem() {}

void WaterCleaningSystem::update(float dt) {
	for (auto& e : entities) {
		auto cleanC = e->getComponent<WaterCleaningComponent>();
		auto transC = e->getComponent<TransformComponent>();
		if (cleanC && cleanC->isCleaning && transC) {
			auto dir = glm::vec3(glm::sin(transC->getRotations().y), 0.f, glm::cos(transC->getRotations().y));
			glm::ivec3 posOffset, negOffset;
			posOffset = glm::ivec3(1, 1, 1);
			negOffset = glm::ivec3(-1, 0, -1);
			auto velNorm = glm::normalize(dir);
			if (velNorm.x > 0) {
				posOffset.x += int(velNorm.x * 3.f) + 1;
			} else {
				negOffset.x += int(velNorm.x * 3.f) - 1;
			}

			if (velNorm.z > 0) {
				posOffset.z += int(velNorm.z * 3.f) + 1;
			} else {
				negOffset.z += int(velNorm.z * 3.f) - 1;
			}

			float amountOfWaterRemoved = m_rendererWrapperRef->removeWaterPoint(transC->getTranslation(), posOffset, negOffset);
			amountOfWaterRemoved *= 0.00392156862f;
			cleanC->amountCleaned += amountOfWaterRemoved;
			if (cleanC->amountCleaned > m_powerUpThreshold) {
				// Spawn power-up here
			}
		}
	}
}
