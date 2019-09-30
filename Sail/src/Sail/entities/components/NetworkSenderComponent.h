#pragma once
#include "Component.h"
#include "Sail/netcode/NetworkedStructs.h"


class NetworSenderComponent : public Component<NetworSenderComponent> {
public:
	// type will decide which components are modified for the entity this component belongs to
	// The ownerID is the ID of the player and is used to create a unique ID for the network component
	NetworSenderComponent(Netcode::NetworkDataType type, unsigned char ownerID);
	~NetworSenderComponent();

	// should be ID and type

	
	//Netcode::NetcodeData data;
};
