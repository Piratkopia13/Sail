#pragma once
#include "Component.h"
#include "../src/Sail/netcode/NetworkedStructs.h"

class LocalOwnerComponent : public Component<LocalOwnerComponent> {
public:
	LocalOwnerComponent() { ; }
	LocalOwnerComponent(Netcode::CompID netEntityId_) : netEntityID(netEntityId_) {	}
	~LocalOwnerComponent() { }

	Netcode::CompID netEntityID;
};