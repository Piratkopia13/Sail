#include "pch.h"
#include "Network/NetworkModule.hpp"
#include "NetworkWrapper.h"

NetworkWrapper::NetworkWrapper()
{
	m_network = new Network();
	m_network->setupHost(54000);
}

NetworkWrapper::~NetworkWrapper()
{
	delete m_network;
}

bool NetworkWrapper::Host(int port)
{
	return false;
}

bool NetworkWrapper::ConnectToIP(const char* ip, int port)
{
	return false;
}

void NetworkWrapper::SendChat(std::string msg)
{
}

void NetworkWrapper::CheckForPackages(NetworkEvent nEvent) {

}
