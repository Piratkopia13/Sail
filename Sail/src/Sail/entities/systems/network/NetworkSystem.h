#pragma once

#include "Sail.h"

#include "Network/NWrapper.h"
#include "../BaseComponentSystem.h"
#include "structPackages/StructPackage.h"

class NetworkSerializedPackageEvent;

class NetworkSystem : public BaseComponentSystem {
public:
	NetworkSystem();
	virtual ~NetworkSystem() {}

	virtual void update(float dt) = 0;
	// Constructor initializes pNWrapper
	void initialize(Entity* playerEntity);

	virtual bool onSerializedPackageRecieved(NetworkSerializedPackageEvent& event) = 0;

protected:
	Entity* m_playerEntity = nullptr;
	NWrapper* m_network = nullptr;
	StructPackage* *m_arr_structs = nullptr;

	void initalizeStructPackages();
	
	/*
		Parses the deserialized package into m_arr_structs where
		each non-deterministic entity's (Those with an attached network component)
		data can be fetched from after this function has been called.
	*/
	void parsePackage(std::string& deSerializedData);





	
};

