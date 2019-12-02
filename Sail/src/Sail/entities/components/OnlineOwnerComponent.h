#pragma once

#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class OnlineOwnerComponent : public Component<OnlineOwnerComponent> {
public:
	OnlineOwnerComponent(Netcode::ComponentID netEntityID);
	~OnlineOwnerComponent();

	Netcode::ComponentID netEntityID;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
	void imguiRender(Entity** selected) {
		ImGui::Text(std::string("netEntityID: " + std::to_string(netEntityID)).c_str());
	}
#endif
};

