#include "pch.h"
#include "NWrapper.h"
#include "Network/NetworkModule.hpp"


NWrapper::NWrapper(Network* pNetwork) {
	this->initialize(pNetwork);
}

NWrapper::~NWrapper() {
	this->shutDown();
}

void NWrapper::initialize(Network* pNetwork) {
	m_network = pNetwork;
	m_app = Application::getInstance();
}

void NWrapper::shutDown() {
	// NO DELETION OF NETWORK, it is the responsibility of the NWrapperSingleton!
	m_network->shutdown();
}

void NWrapper::handleNetworkEvents(NetworkEvent nEvent) {
	switch (nEvent.eventType) {
	case NETWORK_EVENT_TYPE::NETWORK_ERROR:
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_ESTABLISHED:
		playerJoined(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_CLOSED:
		playerDisconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::CONNECTION_RE_ESTABLISHED:
		playerReconnected(nEvent.clientID);
		break;
	case NETWORK_EVENT_TYPE::MSG_RECEIVED:
		decodeMessage(nEvent);
		break;
	default:
		break;
	}
}

TCP_CONNECTION_ID NWrapper::parseID(std::string& data) {
	if (data.size() > 63) {
		return 0;
	}
	if (data.size() < 1) {
		return 0;
	}
	else {
		// Remove opening ':' / '?' marker.
		data.erase(0, 1);

		std::string id_string = "";
		int lastIndex;
		for (lastIndex = 0; lastIndex < data.size(); lastIndex++) {
			if (data[lastIndex] == '\0' || data[lastIndex] == ':') {
				break;
			}
			else {
				id_string += data[lastIndex];
			}
		}

		data.erase(0, lastIndex);
		if (id_string != "") {
			return stoi(id_string);
		}
		else {
			return 0;
		}

	}
}

std::string NWrapper::parseName(std::string& data) {
	if (data.size() < 1) {
		return data;
	}
	else {
		// Remove first ':' marker
		data.erase(0, 1);

		int lastIndex;
		std::string parsedName = "";
		for (lastIndex = 0; lastIndex < MAX_PACKAGE_SIZE; lastIndex++) {
			if (data[lastIndex] == ':') { // Does parseID also remove the last ':'? no?
				break;
			}
			else {
				parsedName += data[lastIndex];
			}
		}

		data.erase(0, lastIndex);
		return parsedName;
	}
}

Message NWrapper::processChatMessage(std::string& message) {
	std::string remnants = message;
	unsigned int id_m = this->parseID(remnants);
	remnants.erase(0, 1);

	return Message{
		std::to_string(id_m),
		remnants
	};
}



void NWrapper::sendMsg(std::string msg) {
	m_network->send(msg.c_str(), msg.length() + 1);
}

void NWrapper::checkForPackages() {
	m_network->checkForPackages(*this);
}

void NWrapper::sendMsgAllClients(std::string msg) {
	m_network->send(msg.c_str(), msg.length() + 1, -1);
}

void NWrapper::sendChatAllClients(std::string msg) {
	msg = std::string("m") + msg;
	m_network->send(msg.c_str(), msg.length() + 1, -1);
}

void NWrapper::sendSerializedData(std::string data) {
	data = std::string("s") + data;
	m_network->send(data.c_str(), data.length(), -1);
}
