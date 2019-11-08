#include "pch.h"
#include "TeamColorSystem.h"

#include "..//..//Entity.h"
#include "Sail/entities/components/TeamComponent.h"
#include "Sail/entities/components/ModelComponent.h"
#include "Sail/graphics/geometry/PBRMaterial.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/Application.h"

TeamColorSystem::TeamColorSystem() {
	registerComponent<TeamComponent>(true, true, false);
	registerComponent<ModelComponent>(true, true, true);

}

TeamColorSystem::~TeamColorSystem() {
}

void TeamColorSystem::update(float dt) {
	for (auto& e : entities) {
		ModelComponent* model = e->getComponent<ModelComponent>();
		TeamComponent* tc = e->getComponent<TeamComponent>();

		model->teamColorID = tc->team;
	}
}

glm::vec4 TeamColorSystem::getTeamColor(int teamID) {
	float f = (teamID / 12.0f) * glm::two_pi<float>();
	return glm::vec4(abs(cos(f * 2)), 1 - abs(cos(f * 1.4)), abs(sin(f * 1.1f)), 1.0f);
}
