#pragma once
#include "..//BaseComponentSystem.h"

class TeamColorSystem final : public BaseComponentSystem {
public:
	TeamColorSystem();
	~TeamColorSystem();

	void update(float dt) override;

	static glm::vec4 getTeamColor(int teamID);
};
