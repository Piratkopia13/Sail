#pragma once

#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class OnlinePlayerComponent : public Component<OnlinePlayerComponent> {
public:
	OnlinePlayerComponent(Netcode::NetworkObjectID netEntityID);
	~OnlinePlayerComponent();

	Netcode::NetworkObjectID netEntityID;
};





