#pragma once
#include "Component.h"
#include "Sail/netcode/NetworkedStructs.h"

class NetworkSenderComponent : public Component<NetworkSenderComponent> {
public:
	// type will decide which components are modified for the entity this component belongs to
	// The ownerID is the ID of the player and is used to create a unique ID for the network component
	NetworkSenderComponent(Netcode::NetworkDataType dataType, Netcode::NetworkEntityType entityType, unsigned char ownerID);
	NetworkSenderComponent(Netcode::NetworkDataType dataType, Netcode::NetworkEntityType entityType, Netcode::NetworkObjectID objectID);
	~NetworkSenderComponent();


	// should be ID and type
	Netcode::NetworkObjectID m_id;
	Netcode::NetworkEntityType m_entityType;
	// Might be changed to a list of types if there are multiple components per entity one wants to
	// send over the network
	Netcode::NetworkDataType m_dataType;
};
