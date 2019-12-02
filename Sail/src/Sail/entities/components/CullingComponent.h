#pragma once
#include "Component.h"

class CullingComponent : public Component<CullingComponent> {
public:
	CullingComponent() { }
	~CullingComponent() { }

	bool isVisible = true;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
	void imguiRender(Entity** selected) {
		ImGui::Columns(2);
		ImGui::Checkbox("##isVisible", &isVisible); ImGui::NextColumn();
		ImGui::Text("isVisible"); 
		ImGui::Columns(1);
	}
#endif

};