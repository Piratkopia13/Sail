#pragma once
#include "..//BaseComponentSystem.h"

class BoundingboxSubmitSystem final : public BaseComponentSystem {
public:
	BoundingboxSubmitSystem();
	~BoundingboxSubmitSystem();

	void toggleHitboxes();
	void submitAll();
#ifdef DEVELOPMENT
	void imguiPrint(Entity** selectedEntity = nullptr) {
		ImGui::Columns(2);
		ImGui::Checkbox("##m_renderHitBoxes", &m_renderHitBoxes); ImGui::NextColumn();
		ImGui::Text(std::string("Velocity").c_str()); ImGui::NextColumn();
		ImGui::Columns(1);
	}
#endif
private:
	bool m_renderHitBoxes;
};
