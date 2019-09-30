#pragma once
#include "Component.h"
#include "Sail/netcode/NetworkedStructs.h"


class NetworkReceiverComponent : public Component<NetworkReceiverComponent> {
public:
	// New receiver components are only created when the application receives NetcodeData with a type that should be
	// continuously updated over the network and with a unique ID that they don't have a NetworkReceiverComponent for yet.
	NetworkReceiverComponent(Netcode::NetworkObjectID id, Netcode::NetworkEntityType type);
	~NetworkReceiverComponent();

	// should only be ID
	Netcode::NetworkObjectID m_id;
	Netcode::NetworkEntityType m_entityType;
};
