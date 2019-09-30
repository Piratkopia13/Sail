#pragma once

#include "ArchiveHelperFunctions.h"


namespace Netcode {
	typedef unsigned __int32 NetworkObjectID;
	
	// Global counter
	extern NetworkObjectID gNetworkIDCounter;
	
	static NetworkObjectID createNetworkID()    { return ++gNetworkIDCounter; }
	static NetworkObjectID nrOfNetworkObjects() { return gNetworkIDCounter; }


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





	// Same type will be used by both sender and receiver
	enum NetworkDataType : __int32 {
		MODIFY_TRANSFORM = 0,
		SHOOT_PROJECTILE = 1,

	};

	struct NetcodeData {
		NetworkObjectID objectID;
		NetworkDataType type;
		union {
			ModifyTransform transform;
			ShootProjectile shootProjectile;
			// ...
		};
	};
}

