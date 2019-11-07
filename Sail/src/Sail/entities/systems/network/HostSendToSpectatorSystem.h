#pragma once

#include "../BaseComponentSystem.h"
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"
#include "Sail/netcode/NetworkedStructs.h"

#include "Sail/netcode/ArchiveTypes.h"
#include "Sail/netcode/NetcodeTypes.h"

class Entity;
class MessageType;
class NetworkReceiverSystem;
struct NetworkSenderEvent;

class HostSendToSpectatorSystem : public BaseComponentSystem {
public:
	HostSendToSpectatorSystem();
	~HostSendToSpectatorSystem();

	void init(Netcode::PlayerID playerID);

	void sendEntityCreationPackage(Netcode::PlayerID toPlayer) const;
private:
	Netcode::PlayerID m_playerID;
};
