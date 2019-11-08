#pragma once
#include "Component.h"

class InsanityComponent : public Component<InsanityComponent> {
public:
	InsanityComponent() {}
	~InsanityComponent() {}

	float insanityValue;

#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		ImGui::Columns(2);
		if (ImGui::DragFloat("##INSANITY", &insanityValue, 0.01, 0, 1)) {

		}
		ImGui::NextColumn();
		ImGui::Text(std::string("InsanityValue").c_str());
	}
#endif
};