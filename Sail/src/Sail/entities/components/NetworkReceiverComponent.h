#pragma once
#include "Component.h"

typedef unsigned __int32 NetworkObjectID;

// Atomic??
extern NetworkObjectID global_networkObjectID;


// Same type will be used by both sender and receiver
enum NetworkDataType : __int32 {
	MODIFY_TRANSPOSE,
	
};


class NetworkReceiverComponent : public Component<NetworkReceiverComponent> {
public:
	NetworkReceiverComponent(unsigned char ownerID);
	~NetworkReceiverComponent();



	// ID first byte will be the player ID, the rest is a 
	NetworkObjectID objectID = 0;
	NetworkDataType type = MODIFY_TRANSPOSE;




	// TODO: Move to some .h file that NetworkSenderComponent has access too as well.
	static NetworkObjectID createNetworkID() {
		return ++global_networkObjectID;
	}

	static NetworkObjectID nrOfNetworkObjects() {
		return global_networkObjectID;
	}
	
};


/*
in system when reading

ar(nrOfObjects)
for(nrOfObjects) {
	ar(objectID, type)
	if (type == CREATE) {
		e = createEntity(type)
		e.deserialize(ar)
	} else {
		find (entity == objectID) {
			entity.deserialize(ar)
		}
	}
}


*/