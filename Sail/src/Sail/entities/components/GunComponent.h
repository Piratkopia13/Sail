#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Model;

enum GunState {
	STARTING,
	LOOPING,
	ENDING,
	STANDBY
};

class GunComponent : public Component<GunComponent> {
public:
	GunComponent();
	~GunComponent() {};

	void setFiring(glm::vec3 pos, glm::vec3 dir);

	glm::vec3 position;
	glm::vec3 direction;

	float projectileSpawnTimer;
	float gunOverloadTimer;
	float m_projectileSpawnCooldown;
	float m_gunOverloadCooldown;

	float projectileSpeed;

	float gunOverloadvalue;
	float gunOverloadThreshold;

	bool firing;

	// Used to manage sound in gunsystem.
	bool firingContinuously = false;
	GunState state = GunState::STANDBY;
#ifdef DEVELOPMENT
	void imguiRender() {
		ImGui::Columns(2);
		if (ImGui::DragFloat("##aspeeed", &projectileSpeed, 0.1f)) {
		}ImGui::NextColumn();
		ImGui::Text("Speed"); 
		ImGui::Columns(1);
		
		
	}
#endif
private:

	float m_projectileSpawnLimit = .3f;
};
