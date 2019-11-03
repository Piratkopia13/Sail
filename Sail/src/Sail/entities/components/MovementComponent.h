#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class MovementComponent final : public Component<MovementComponent> {
public:
	MovementComponent() {}
	~MovementComponent() {}

	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 relVel = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 constantAcceleration = glm::vec3(0.0f);
	glm::vec3 accelerationToAdd = glm::vec3(0.0f);

	glm::vec3 oldVelocity = glm::vec3(0.0f);
	glm::vec3 oldMovement = glm::vec3(0.0f);

	float airDrag = 1.0f;

	float updateableDt = 0.0f;

#ifdef DEVELOPMENT
	void imguiRender() {
		ImGui::Columns(2);
		ImGui::DragFloat3("##VEL", &velocity.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("Velocity").c_str()); ImGui::NextColumn();
		glm::vec3 tempRel = relVel;
		ImGui::DragFloat3("##RELVEL", &tempRel.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("RelVelocity").c_str()); ImGui::NextColumn();

		ImGui::DragFloat3("##ROTATIONVEL", &rotation.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("rotationVel").c_str()); ImGui::NextColumn();

		ImGui::DragFloat3("##constantAcceleration", &constantAcceleration.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("constantAcceleration").c_str()); ImGui::NextColumn();

		ImGui::DragFloat3("##oldVelocity", &oldVelocity.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("oldVelocity").c_str()); ImGui::NextColumn();

		ImGui::DragFloat3("##oldMovement", &oldMovement.x, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("oldMovement").c_str()); ImGui::NextColumn();		
		
		ImGui::DragFloat("##airDrag", &airDrag, 0.1f); ImGui::NextColumn();
		ImGui::Text(std::string("airDrag").c_str()); ImGui::NextColumn();



		ImGui::Columns(1);



	}
#endif
};