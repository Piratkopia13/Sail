#include "pch.h"
#include "NetworkReceiverSystemHost.h"
#include "../NetworkSenderSystem.h"

NetworkReceiverSystemHost::NetworkReceiverSystemHost() {

}

NetworkReceiverSystemHost::~NetworkReceiverSystemHost() {

}

void NetworkReceiverSystemHost::pushDataToBuffer(std::string data) {
	std::scoped_lock lock(m_bufferLock);
	m_incomingDataBuffer.push(data);

	// The host will also save the data in the sender system so that it can be forwarded to all other clients
	m_netSendSysPtr->pushDataToBuffer(data);
}