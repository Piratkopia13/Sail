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
	const unsigned int getByteSize() const override {
		/* TODO: Fix component size */
		return sizeof(*this);
	}
	void imguiRender(Entity** selected) {
		ImGui::Columns(2);
		ImGui::DragFloat("##drag", &drag, 0.1f); ImGui::NextColumn();
		ImGui::Text("drag"); ImGui::NextColumn();

		ImGui::DragFloat("##bounciness", &bounciness, 0.1f); ImGui::NextColumn();
		ImGui::Text("bounciness"); ImGui::NextColumn();

		ImGui::DragFloat("##padding", &padding, 0.1f); ImGui::NextColumn();
		ImGui::Text("padding"); ImGui::NextColumn();

		ImGui::Checkbox("##onGround", &onGround); ImGui::NextColumn();
		ImGui::Text("onGround"); ImGui::NextColumn();

		ImGui::Checkbox("##doSimpleCollisions", &doSimpleCollisions); ImGui::NextColumn();
		ImGui::Text("doSimpleCollisions"); ImGui::NextColumn();
		ImGui::Columns(1);

		ImGui::Separator();

		for (auto& c : collisions) {
			if (ImGui::Selectable(std::string(c.entity->getName() + "(" + std::to_string(c.entity->getID()) + ")").c_str(), (*selected) == c.entity)) {
				SAIL_LOG("Tried to switch");
				*selected = c.entity;
				return;
			}
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
