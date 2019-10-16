#pragma once
#include "ArchiveHelperFunctions.h"

namespace Netcode {
	typedef unsigned __int32 NetworkObjectID;
	
	// Global counter
	extern NetworkObjectID gNetworkIDCounter;
	
	static NetworkObjectID createNetworkID()    { return ++gNetworkIDCounter; }
	static NetworkObjectID nrOfNetworkObjects() { return gNetworkIDCounter; }


	/*
	  Enums for the kind of entities/data that will be sent over the network,
	  will just be used to read to/write from in the NetworkSystems
	*/


	// Pre-defined entity types so that other players know which entity to create
	enum EntityType : __int32 {
		PLAYER_ENTITY = 1,
		MECHA_ENTITY = 2,
	};

	// The message type decides how the subsequent data will be parsed and used
	enum MessageType : __int32 {
		CREATE_NETWORKED_ENTITY = 1,
		MODIFY_TRANSFORM = 2,
		SPAWN_PROJECTILE = 3,
		ROTATION_TRANSFORM = 4,
		PLAYER_JUMPED = 5,
		WATER_HIT_PLAYER = 6,
		PLAYER_DIED = 7,
		MATCH_ENDED = 8,
		EMPTY = 69
	};

	/*
	  Structs for the kind of data that will be sent/
	  They map to the message types as if the message consists of a tagged union

	  Logical structure of a message:
	  
	  struct NetworkMessage {
	      NetworkObjectID objectID;
	      MessageType     type;
	      union {
	          ModifyTransform transform;
	          SpawnProjectile spawnProjectile;
	          ...
	      }
	  }
	*/

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

	// ROTATION_TRANSFORM
	struct RotationTransform {
		glm::vec3 rotation;

		template <class Archive>
		void save(Archive& ar) const {
			Archive::serializeVec3(ar, position);
		}

		template <class Archive>
		void load(Archive& ar) {
			Archive::serializeVec3(ar, position);
		}
	};

	// SPAWN_PROJECTILE
	struct SpawnProjectile {
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


	class MessageData {
	public:
		MessageData() {}
		virtual ~MessageData() {}
	};

	class MessageDataProjectile : public MessageData {
	public:
		MessageDataProjectile(glm::vec3 translation_, glm::vec3 velocity_)
			: translation(translation_), velocity(velocity_)
		{}
		virtual ~MessageDataProjectile() {}

		glm::vec3 translation;
		glm::vec3 velocity;
	};

	class MessageDataWaterHitPlayer : public MessageData {
	public:
		MessageDataWaterHitPlayer(Netcode::NetworkObjectID id)
			: playerWhoWasHitID(id)
		{}
		~MessageDataWaterHitPlayer() {}

		Netcode::NetworkObjectID playerWhoWasHitID;
	};
}

