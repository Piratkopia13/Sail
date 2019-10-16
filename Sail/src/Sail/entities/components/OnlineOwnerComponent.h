#pragma once

#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class OnlineOwnerComponent : public Component<OnlineOwnerComponent> {
public:
	OnlineOwnerComponent(Netcode::NetworkObjectID netEntityID);
	~OnlineOwnerComponent();

	Netcode::NetworkObjectID netEntityID;
};

