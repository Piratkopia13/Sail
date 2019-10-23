#pragma once
#include "ArchiveHelperFunctions.h"
#include "NetcodeTypes.h"

#include <atomic>

namespace Netcode {
	// Global counter
	extern std::atomic<ComponentID> gNetworkIDCounter;
	static ComponentID createNetworkID()    { return ++gNetworkIDCounter; }
	static ComponentID nrOfNetworkObjects() { return gNetworkIDCounter; }


	// Used to signify NetworkMessages sent Internally
	static constexpr PlayerID MESSAGE_FROM_SELF_ID = 255;

	// ComponentID has 32 bits and the first 8 are the PlayerID of the owner which
	// can be extracted by shifting the ComponentID 18 bits to the right.
	static constexpr ComponentID SHIFT_AMOUNT = 18;


	// Generates a unique ID for a NetworkSenderComponent based on the player's PlayerID
	static ComponentID generateUniqueComponentID(PlayerID ownerID) {
		return (createNetworkID() | (static_cast<ComponentID>(ownerID) << SHIFT_AMOUNT));
	}
	// Extract the PlayerID of the owner of a NetworkComponent from the component's ID
	static constexpr PlayerID getComponentOwner(ComponentID componentID) {
		return static_cast<PlayerID>(componentID >> SHIFT_AMOUNT);
	}


	/*
	  Enums for the kind of entities/data that will be sent over the network,
	  will just be used to read to/write from in the NetworkSystems
	*/


	// Pre-defined entity types so that other players know which entity to create
	enum class EntityType : __int32 {
		PLAYER_ENTITY = 1,
		MECHA_ENTITY = 2,
	};

	// TODO: should be one message type for tracked entities and one for events
	// The message type decides how the subsequent data will be parsed and used
	enum class MessageType : __int32 {
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
		MessageDataWaterHitPlayer(Netcode::ComponentID id)
			: playerWhoWasHitID(id)
		{}
		~MessageDataWaterHitPlayer() {}

		Netcode::ComponentID playerWhoWasHitID;
	};

	class MessageDataPlayerDied : public MessageData {
	public:
		MessageDataPlayerDied(Netcode::ComponentID id) : playerWhoDied(id) {}
		~MessageDataPlayerDied() {}
		Netcode::ComponentID playerWhoDied;
	};

	class MessageDataCandleHeldState : public MessageData {
	public:
		MessageDataCandleHeldState(Netcode::ComponentID id, bool b, glm::vec3 pos) : candleOwnerID(id), isHeld(b), candlePos(pos) {}
		~MessageDataCandleHeldState() {}
		Netcode::ComponentID candleOwnerID;
		bool isHeld;
		glm::vec3 candlePos;
	};

	class MessageDataPlayerDisconnect : public MessageData {
	public:
		MessageDataPlayerDisconnect(PlayerID id) : playerID(id) {}
		~MessageDataPlayerDisconnect() {}
		PlayerID playerID;
	};
}

