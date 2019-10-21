#include "pch.h"
#include "NetworkReceiverSystemClient.h"

NetworkReceiverSystemClient::NetworkReceiverSystemClient() {

}

NetworkReceiverSystemClient::~NetworkReceiverSystemClient() {

}

void NetworkReceiverSystemClient::pushDataToBuffer(std::string data) {
	std::scoped_lock lock(m_bufferLock);
	m_incomingDataBuffer.push(data);
}
