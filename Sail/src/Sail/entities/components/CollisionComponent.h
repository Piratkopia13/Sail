#pragma once
#include "Component.h"
#include "..//..//Physics/Octree.h"

class CollisionComponent : public Component<CollisionComponent> {
public:
	CollisionComponent(bool simpleCollisions = false);
	~CollisionComponent();

	float drag;
	float bounciness;
	float padding;
	bool onGround;
	bool doSimpleCollisions;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions

#ifdef DEVELOPMENT
	void imguiRender() {
		ImGui::Columns(2);
		ImGui::DragFloat("##drag", &drag); ImGui::NextColumn();
		ImGui::Text("drag"); ImGui::NextColumn();

		ImGui::DragFloat("##bounciness", &bounciness); ImGui::NextColumn();
		ImGui::Text("bounciness"); ImGui::NextColumn();

		ImGui::DragFloat("##padding", &padding); ImGui::NextColumn();
		ImGui::Text("padding"); ImGui::NextColumn();

		ImGui::Checkbox("##onGround", &onGround); ImGui::NextColumn();
		ImGui::Text("onGround"); ImGui::NextColumn();

		ImGui::Checkbox("##doSimpleCollisions", &doSimpleCollisions); ImGui::NextColumn();
		ImGui::Text("doSimpleCollisions"); ImGui::NextColumn();
		ImGui::Columns(1);

		ImGui::Separator();

		for (auto& c : collisions) {
			ImGui::Text(std::string(c.entity->getName()+"("+std::to_string(c.entity->getID())+")").c_str());
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(std::string("axis:    " + Utils::toStr(c.intersectionAxis)).c_str());
				ImGui::Text(std::string("position:" + Utils::toStr(c.intersectionPosition)).c_str());
				ImGui::EndTooltip();
			}

		}





	}
#endif

};