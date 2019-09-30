#pragma once

#include "ArchiveHelperFunctions.h"


namespace Netcode {
	typedef unsigned __int32 NetworkObjectID;
	
	// Global counter
	extern NetworkObjectID gNetworkIDCounter;
	
	static NetworkObjectID createNetworkID()    { return ++gNetworkIDCounter; }
	static NetworkObjectID nrOfNetworkObjects() { return gNetworkIDCounter; }


	//// Structs for the kind of data that will be sent over the network,
	//// will just be used to read to/write from in the NetworkSystems

	enum NetworkEntityType : __int32 {
		PLAYER_ENTITY = 1,
		MECHA_ENTITY = 2,
	};

	// Same type will be used by both sender and receiver
	enum NetworkDataType : __int32 {
		CREATE_NETWORKED_ENTITY = 1,
		MODIFY_TRANSFORM = 2,
		SPAWN_PROJECTILE = 3,

	};

	// MODIFY_TRANSFORM
	struct ModifyTransform {
		glm::vec3 transform;

		template <class Archive>
		void save(Archive& ar) const {
			Archive::serializeVec3(ar, position);
		}

		template <class Archive>
		void load(Archive& ar) {
			Archive::serializeVec3(ar, position);
		}
	};

	// SHOOT_PROJECTILE
	struct ShootProjectile {
		glm::vec3 position;
		glm::quat rotation;
		// velocity or something maybe?
	
		template <class Archive>
		void save(Archive& ar) const {
			Archive::serializeVec3(ar, position);
			Archive::serealizeQuat(ar, rotation);
		}

		template <class Archive>
		void load(Archive& ar) {
			Archive::serealizeQuat(ar, rotation);
			Archive::serializeVec3(ar, position);
		}
	};




	// This is logically how the data will be sent/received.
	/*
	struct NetcodeData {
		NetworkObjectID objectID;
		NetworkDataType type;
		union {
			ModifyTransform transform;
			ShootProjectile shootProjectile;
			// ...
		};
	};
	*/
}

