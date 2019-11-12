#pragma once

#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class OnlineOwnerComponent : public Component<OnlineOwnerComponent> {
public:
	OnlineOwnerComponent(Netcode::CompID netEntityID);
	~OnlineOwnerComponent();

	Netcode::CompID netEntityID;

#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		ImGui::Text(std::string("netEntityID: " + std::to_string(netEntityID)).c_str());
	}
#endif
};

