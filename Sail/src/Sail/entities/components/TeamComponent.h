#pragma once
#include "Component.h"
#include "Sail/entities/systems/Gameplay/TeamColorSystem.h"
//class Model;

class TeamComponent : public Component<TeamComponent> {
public:
	TeamComponent(int team = 0){
		this->team = team;
	}
	~TeamComponent() {}

	int team;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
	void imguiRender(Entity** selected) {
		ImGui::Columns(2);
		if (ImGui::DragInt("##TEAM", &team, 0.1, 0 , 11)) {
		
		}
		ImGui::NextColumn();
		ImGui::Text(std::string("TeamID").c_str());

		ImGui::NextColumn();
		ImVec4 col;
		glm::vec4 c = TeamColorSystem::getTeamColor(team);
		col.x = c.x;
		col.y = c.y;
		col.z = c.z;
		col.w = c.a;
		ImGui::Text((std::string("(") + std::to_string(col.x) + ", " + std::to_string(col.y) + ", " + std::to_string(col.z) + ")").c_str());

		ImGui::NextColumn();
		ImGui::Text(std::string("Team Color").c_str());
	}
#endif
};