#include "pch.h"
#include "LightSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/components/LightListComponent.h"

#include "Sail/graphics/light/LightSetup.h"

#ifdef _DEBUG
#include "Sail/graphics/camera/PerspectiveCamera.h"
#endif

LightSystem::LightSystem() : BaseComponentSystem() {
	registerComponent<LightComponent>(true, true, true);
	/* Seems like it reads from it sometimes */
	registerComponent<LightListComponent>(false, true, false);
}

LightSystem::~LightSystem() 
{}


void LightSystem::update(float dt) 
{}

//check and update all lights for all entities
void LightSystem::updateLights(LightSetup* lightSetup) {
	for (auto e : entities) {
		LightComponent* lc = e->getComponent<LightComponent>();
		if (lc) {
			lightSetup->addPointLight(lc->getPointLight());
		}

		LightListComponent* llc = e->getComponent<LightListComponent>();
		if (llc) {
			for (auto& light : llc->getLightList()) {
				lightSetup->addPointLight(light);
			}
		}
	}
}

#ifdef _DEBUG
void LightSystem::setDebugLightListEntity(std::string name) {
	for (auto e : entities) {
		if (e->getName() == name) {
			e->addComponent<LightListComponent>();
			m_debugLightListEntity = e;
			break;
		}
	}
}

void LightSystem::addPointLightToDebugEntity(LightSetup* lightSetup, PerspectiveCamera* cam) {
	PointLight pl;
	pl.setColor(glm::vec3(Utils::rnd(), Utils::rnd(), Utils::rnd()));
	pl.setPosition(cam->getPosition());
	pl.setAttenuation(.0f, 0.1f, 0.02f);
	
	if (m_debugLightListEntity) {
		LightListComponent* llc = m_debugLightListEntity->getComponent<LightListComponent>();
		if (llc) {
			llc->getLightList().push_back(pl);
		}
	} else {
		Logger::Warning("m_debugLightListEntity not setup, point light could not be added.");
	}
}

void LightSystem::removePointLightFromDebugEntity() {
	// Remove the most recently added light from the lightList
	if (m_debugLightListEntity) {
		LightListComponent* llc = m_debugLightListEntity->getComponent<LightListComponent>();
		if (llc && llc->getLightList().size() > 0) {
			llc->getLightList().erase(llc->getLightList().begin());
		}
	}
}
#endif

