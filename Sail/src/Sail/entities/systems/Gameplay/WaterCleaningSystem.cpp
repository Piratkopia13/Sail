#include "pch.h"
#include "WaterCleaningSystem.h"

#include <Sail.h>

WaterCleaningSystem::WaterCleaningSystem()
	: m_rendererWrapperRef(Application::getInstance()->getRenderWrapper()) 
{
	registerComponent<WaterCleaningComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
}

WaterCleaningSystem::~WaterCleaningSystem() {}

void WaterCleaningSystem::update(float dt) {
	for (auto& e : entities) {
		auto cleanC = e->getComponent<WaterCleaningComponent>();
		auto transC = e->getComponent<TransformComponent>();
		if (cleanC && cleanC->isCleaning && transC) {
			auto dir = glm::vec3(glm::cos(transC->getRotations().y), 0.f, glm::sin(transC->getRotations().y));
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

			m_rendererWrapperRef->removeWaterPoint(transC->getTranslation(), posOffset, negOffset);
		}
	}
}
