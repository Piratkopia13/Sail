#pragma once

#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class OnlineOwnerComponent : public Component<OnlineOwnerComponent> {
public:
	OnlineOwnerComponent(Netcode::NetworkComponentID netEntityID);
	~OnlineOwnerComponent();

	Netcode::NetworkComponentID netEntityID;
};

