#include "pch.h"
#include "NetworkReceiverComponent.h"

NetworkObjectID global_networkObjectID = 0;

NetworkReceiverComponent::NetworkReceiverComponent(unsigned char ownerID) {
	// Set the first byte of the object ID as the ownerID

	objectID = createNetworkID() & (static_cast<NetworkObjectID>(ownerID) << 18);
}

NetworkReceiverComponent::~NetworkReceiverComponent() {
	
}
