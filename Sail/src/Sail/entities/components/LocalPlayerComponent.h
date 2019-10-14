#pragma once
#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class LocalPlayerComponent : public Component<LocalPlayerComponent> {
public:
	LocalPlayerComponent() { ; }
	LocalPlayerComponent(Netcode::NetworkObjectID netEntityId_) : netEntityID(netEntityId_) {	}
	~LocalPlayerComponent() { }

	Netcode::NetworkObjectID netEntityID;
};