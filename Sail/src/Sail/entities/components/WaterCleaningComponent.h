#pragma once
#include "Component.h"

class WaterCleaningComponent : public Component<WaterCleaningComponent> {
public:
	WaterCleaningComponent() {}
	~WaterCleaningComponent() {}

	bool isCleaning = true;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
	void imguiRender(Entity** selected) {
		ImGui::Columns(2);
		ImGui::Checkbox("##isCleaning", &isCleaning); ImGui::NextColumn();
		ImGui::Text("isCleaning");
		ImGui::Columns(1);
	}
#endif

};