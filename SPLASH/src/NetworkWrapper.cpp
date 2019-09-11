#include "pch.h"
#include "Network/NetworkModule.hpp"
#include "NetworkWrapper.h"

NetworkWrapper::NetworkWrapper() {
	m_network = new Network();
	//m_network->setupHost(54000);
}

NetworkWrapper::~NetworkWrapper() {
	m_network->shutdown();
	delete m_network;

}

bool NetworkWrapper::Host(int port) {
	if (!m_network->isServer())
	{
		//return m_network->setupHost(port);
	}
	return m_network->setupHost(port);
}

bool NetworkWrapper::ConnectToIP(const char* ip, int port) {
	return m_network->setupClient(ip, port);
}

void NetworkWrapper::SendChat(char* msg) {
	m_network->send(msg, sizeof(msg));
}

void NetworkWrapper::SendChatAllClients(char* msg) {
	m_network->send(msg, sizeof(msg), -1);
}

void NetworkWrapper::ProcessPackages(NetworkEvent nEvent) {

	switch (nEvent.eventType)
	{
	case NETWORK_EVENT_TYPE::NETWORK_ERROR:
		break;
	case NETWORK_EVENT_TYPE::CLIENT_JOINED:
		PlayerJoined(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CLIENT_DISCONNECTED:
		PlayerDisconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CLIENT_RECONNECTED:
		PlayerReconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::MSG_RECEIVED:
		DecodeMessage(nEvent);
		break;
	default:
		break;
	}
}

void NetworkWrapper::CheckForPackages() {
	//m_network->checkForPackages(&ProcessPackages);
}

void NetworkWrapper::DecodeMessage(NetworkEvent nEvent) {

	// These will be assigned in the switch case.
	int userID;
	std::string message;
	char* charAsInt[4];

	switch (nEvent.data->msg[0])
	{
	case 'm':
		// handle chat message.
		message = std::string(nEvent.data->msg) + std::string("\n");
		printf(message.c_str());
		// TODO:
		// Print message to the chat window
		break;

	case 'd':
		// Handle disconnects.
		

		for (int i = 0; i < 4; i++) {
			charAsInt[i] = &nEvent.data->msg[1 + i];
		}
		
		userID = reinterpret_cast<int>(&charAsInt);
		// TODO:
		// Remove the user with this ID from the lobby and print out that it disconnected.
		break;

	case 'j':
		// Handle players joining the game. 

		for (int i = 0; i < 4; i++) {
			charAsInt[i] = &nEvent.data->msg[1 + i];
		}

		userID = reinterpret_cast<int>(&charAsInt);
		// TODO:
		// Add this user id to the list of players in the lobby.
		break;

	default:
		printf((std::string("Error: Packet message with key: ") + 
			nEvent.data->msg[0] + "can't be handled. \n").c_str());
		break;
	}

}

void NetworkWrapper::PlayerDisconnected(ConnectionID id) {

	char msg[64];
	int intid = (int)id;
	char* int_asChar = reinterpret_cast<char*>(&intid);

	msg[0] = 'd';
	for (int i = 0; i < 4; i++) {
		msg[i + 1] = int_asChar[i];
	}

	// Send to all clients that soneone disconnected and which id.
	m_network->send(msg ,sizeof(msg) ,-1);

	// Remove the user with this ID from the lobby and print out that it disconnected.
}

void NetworkWrapper::PlayerReconnected(ConnectionID id) {
	/*
		This remains unimplemented.
	*/
}

void NetworkWrapper::PlayerJoined(ConnectionID id) {

	char msg[64];
	int intid = id;
	char* int_asChar = reinterpret_cast<char*>(&intid);

	msg[0] = 'j';
	for (int i = 0; i < 4; i++) {
		msg[i + 1] = int_asChar[i];
	}

	// Send to all clients that someone joined and which id.
	m_network->send(msg, sizeof(msg), -1);

	// Add this user id to the list of players in the lobby.
}
