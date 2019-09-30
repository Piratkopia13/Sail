#include "pch.h"
#include "NetworkReceiverComponent.h"

NetworkReceiverComponent::NetworkReceiverComponent(Netcode::NetworkObjectID id, Netcode::NetworkEntityType type)
	: m_id(id), m_entityType(type)
{}

NetworkReceiverComponent::~NetworkReceiverComponent() 
{}
