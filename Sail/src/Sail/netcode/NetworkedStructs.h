#pragma once
#include "ArchiveHelperFunctions.h"
#include "NetcodeTypes.h"

namespace Netcode {
	// Global counter
	extern NetworkComponentID gNetworkIDCounter;
	
	static NetworkComponentID createNetworkID()    { return ++gNetworkIDCounter; }
	static NetworkComponentID nrOfNetworkObjects() { return gNetworkIDCounter; }

	// Used to signify NetworkMessages sent Internally
	constexpr PlayerID MESSAGE_FROM_SELF_ID = 255;

	/*
	  Enums for the kind of entities/data that will be sent over the network,
	  will just be used to read to/write from in the NetworkSystems
	*/


	// Pre-defined entity types so that other players know which entity to create
	enum EntityType : __int32 {
		PLAYER_ENTITY = 1,
		MECHA_ENTITY = 2,
	};

	// TODO: should be one message type for tracked entities and one for events
	// The message type decides how the subsequent data will be parsed and used
	enum MessageType : __int32 {
		CREATE_NETWORKED_ENTITY = 1,
		MODIFY_TRANSFORM        = 2,
		SPAWN_PROJECTILE        = 3,
		ROTATION_TRANSFORM      = 4,
		PLAYER_JUMPED           = 5,
		WATER_HIT_PLAYER        = 6,
		SET_CANDLE_HEALTH       = 7,
		PLAYER_DIED             = 8,
		PLAYER_DISCONNECT       = 9,
		MATCH_ENDED             = 10,
		CANDLE_HELD_STATE       = 11,
		SEND_ALL_BACK_TO_LOBBY  = 12,
		EMPTY                   = 69
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
			ArchiveHelpers::serializeVec3(ar, position);
		}

		template <class Archive>
		void load(Archive& ar) {
			ArchiveHelpers::serializeVec3(ar, position);
		}
	};

	// ROTATION_TRANSFORM
	struct RotationTransform {
		glm::vec3 rotation;

		template <class Archive>
		void save(Archive& ar) const {
			ArchiveHelpers::serializeVec3(ar, position);
		}

		template <class Archive>
		void load(Archive& ar) {
			ArchiveHelpers::serializeVec3(ar, position);
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
		MessageDataWaterHitPlayer(Netcode::NetworkComponentID id)
			: playerWhoWasHitID(id)
		{}
		~MessageDataWaterHitPlayer() {}

		Netcode::NetworkComponentID playerWhoWasHitID;
	};

	class MessageDataPlayerDied : public MessageData {
	public:
		MessageDataPlayerDied(Netcode::NetworkComponentID id) : playerWhoDied(id) {}
		~MessageDataPlayerDied() {}
		Netcode::NetworkComponentID playerWhoDied;
	};

	class MessageDataCandleHeldState : public MessageData {
	public:
		MessageDataCandleHeldState(Netcode::NetworkComponentID id, bool b, glm::vec3 pos) : candleOwnerID(id), isHeld(b), candlePos(pos) {}
		~MessageDataCandleHeldState() {}
		Netcode::NetworkComponentID candleOwnerID;
		bool isHeld;
		glm::vec3 candlePos;
	};

	class MessageDataPlayerDisconnect : public MessageData {
	public:
		MessageDataPlayerDisconnect(unsigned char id) : playerID(id) {}
		~MessageDataPlayerDisconnect() {}
		unsigned char playerID;
	};
}

