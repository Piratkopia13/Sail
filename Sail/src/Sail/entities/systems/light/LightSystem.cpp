#include "pch.h"
#include "LightSystem.h"

#include "Sail/entities/components/LightComponent.h"

// TODO: couple this system with m_lights in GameState somehow.

LightSystem::LightSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(LightComponent::ID);


}

LightSystem::~LightSystem() {

}

void LightSystem::update(float dt) {
	for (auto e : entities) {



	}

	//std::vector<Entity::SPtr> entities = m_scene.getGameObjectEntities();
	//m_lights.addPointLight(m_playerController.getCandle()->getComponent<LightComponent>()->m_pointLight);
	//for (int i = 0; i < entities.size(); i++) {
	//	if (entities[i]->hasComponent<LightComponent>()) {
	//		m_lights.addPointLight(entities[i]->getComponent<LightComponent>()->m_pointLight);
	//	}
	//	if (entities[i]->hasComponent<LightListComponent>()) {
	//		for (int j = 0; j < entities[i]->getComponent<LightListComponent>()->m_pls.size(); j++) {
	//			m_lights.addPointLight(entities[i]->getComponent<LightListComponent>()->m_pls[j]);
	//		}
	//	}
	//}
}


