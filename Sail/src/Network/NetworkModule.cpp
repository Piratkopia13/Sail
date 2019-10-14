#include "pch.h"
#include "NetworkModule.hpp"
#include <random>
#include "Sail/../../libraries/cereal/archives/portable_binary.hpp"

Network::Network() {}

Network::~Network() {
	shutdown();

	for (int i = 0; i < MAX_AWAITING_PACKAGES; ++i) {
		if (m_awaitingEvents[i].eventType != NETWORK_EVENT_TYPE::HOST_ON_LAN_FOUND && m_awaitingMessages[i].Message.rawMsg != nullptr) {
			delete[] m_awaitingMessages[i].Message.rawMsg;
			m_awaitingMessages[i].Message.rawMsg = nullptr;
		}
	}

	delete[] m_awaitingEvents;
	delete[] m_awaitingMessages;

	WSACleanup();
}

bool Network::initialize()
{
	m_shutdown = false;

	if (m_initializedStatus) {
		return true;
	}

	WSADATA data;
	WORD version = MAKEWORD(2, 2); // use version winsock 2.2
	int status = WSAStartup(version, &data);
	if (status != 0) {
#ifdef DEBUG_NETWORK
		printf("Error starting WSA\n");
#endif // DEBUG_NETWORK
		return false;
	}

	m_awaitingMessages = SAIL_NEW NetworkEventData[MAX_AWAITING_PACKAGES];
	m_awaitingEvents = SAIL_NEW NetworkEvent[MAX_AWAITING_PACKAGES];

	m_initializedStatus = INITIALIZED_STATUS::INITIALIZED;
	m_shutdown = false;

	return true;
}

void Network::checkForPackages(NetworkEventHandler& handler)
{
	bool morePackages = true;
	while (morePackages)
	{
		std::lock_guard<std::mutex> lock(m_mutex_packages);
		if (m_pstart == m_pend) {
			morePackages = false;
			break;
		}
		handler.handleNetworkEvents(m_awaitingEvents[m_pstart]);
		m_pstart = (m_pstart + 1) % MAX_AWAITING_PACKAGES;
	}
}

void Network::checkForPackages(void (*m_callbackfunction)(NetworkEvent)) {
	bool morePackages = true;
	while (morePackages)
	{
		std::lock_guard<std::mutex> lock(m_mutex_packages);
		if (m_pstart == m_pend) {
			morePackages = false;
			break;
		}
		m_callbackfunction(m_awaitingEvents[m_pstart]);
		m_pstart = (m_pstart + 1) % MAX_AWAITING_PACKAGES;
	}
}

bool Network::host(unsigned short port, USHORT hostFlags)
{
	if (m_initializedStatus != INITIALIZED_STATUS::INITIALIZED) {
		return false;
	}

	m_connections.clear();
	m_connections.reserve(128);

	memset(m_serverMetaDesc, 0, sizeof(m_serverMetaDesc));

	m_hostFlags = hostFlags;
	m_hostPort = port;

	m_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //IPPROTO_TCP
	if (m_soc == INVALID_SOCKET) {
#ifdef DEBUG_NETWORK
		printf("Error creating socket\n");
#endif
	}
	m_myAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	m_myAddr.sin_family = AF_INET;
	m_myAddr.sin_port = htons(port);

	if (bind(m_soc, (sockaddr*)& m_myAddr, sizeof(m_myAddr)) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
		printf("Error binding socket\n");
#endif // DEBUG_NETWORK
		return false;
	}

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(bool);

	int iOptVal = 0;
	int iOptLen = sizeof(int);

	setsockopt(m_soc, IPPROTO_TCP, TCP_NODELAY, (char*)& bOptVal, bOptLen);//Prepare the socket to listen
	setsockopt(m_soc, IPPROTO_TCP, SO_SNDBUF, (char*)& iOptVal, iOptLen);//Prepare the socket to listen

	::listen(m_soc, SOMAXCONN);

	m_shutdown = false;
	m_initializedStatus = INITIALIZED_STATUS::IS_SERVER;

	//Start a new thread that will wait for new connections
	m_clientAcceptThread = SAIL_NEW std::thread(&Network::waitForNewConnections, this);
	if (m_hostFlags & (USHORT)HostFlags::ENABLE_LAN_SEARCH_VISIBILITY && !startUDPSocket(m_udp_localbroadcastport)) {
			return false;
	}


	return true;
}

bool Network::join(const char* IP_adress, unsigned short hostport)
{
	if (m_initializedStatus != INITIALIZED_STATUS::INITIALIZED) {
		return false;
	}

	m_soc = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
	if (m_soc == INVALID_SOCKET) {
#ifdef DEBUG_NETWORK
		printf("Error creating socket\n");
#endif
		return false;
	}

	m_myAddr.sin_family = AF_INET;
	m_myAddr.sin_port = htons(hostport);
	inet_pton(AF_INET, IP_adress, &m_myAddr.sin_addr);

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(bool);

	int iOptVal = 0;
	int iOptLen = sizeof(int);

	setsockopt(m_soc, IPPROTO_TCP, TCP_NODELAY, (char*)& bOptVal, bOptLen);//Prepare the socket to listen
	setsockopt(m_soc, IPPROTO_TCP, SO_SNDBUF, (char*)& iOptVal, iOptLen);//Prepare the socket to listen

	//connect needs error checking!
	int conres = connect(m_soc, (sockaddr*)& m_myAddr, sizeof(m_myAddr));
	if (conres == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
		printf("Error connecting to host\n");
#endif
		return false;
	}

	Connection* conn = SAIL_NEW Connection;
	conn->isConnected = true;
	conn->socket = m_soc;
	conn->ip = "";
	conn->port = ntohs(m_myAddr.sin_port);
	conn->id = 0;
	conn->thread = SAIL_NEW std::thread(&Network::listen, this, conn); //Create new listening thread listening for the host
	m_connections[conn->id] = conn;

	m_initializedStatus = INITIALIZED_STATUS::IS_CLIENT;

	return true;
}

bool Network::send(const char* message, size_t size, TCP_CONNECTION_ID receiverID)
{
	if (receiverID == -1 && m_initializedStatus == INITIALIZED_STATUS::IS_SERVER) {
		int success = 0;
		Connection* conn = nullptr;
		for (auto it : m_connections)
		{
			conn = it.second;
			if (send(message, size, conn))
				success++;
		}
		return true;
	}

	// serialize the packet size and place that information at the beginning of the packet
	std::ostringstream os(std::ios::binary);
	{
		cereal::PortableBinaryOutputArchive ar(os);
		ar(size);
	}
	std::string msgSizeString = os.str();

	size_t packetSize = MSG_SIZE_STR_LEN + size;
	char* msg = SAIL_NEW char[packetSize]();
	
	// The message starts with a size_t stating how large the message is
	memcpy(&msg[0], msgSizeString.c_str(), MSG_SIZE_STR_LEN);

	// The rest of msg is the actual message
	memcpy(&msg[MSG_SIZE_STR_LEN], message, size);

	Connection* conn = nullptr;
	{
		std::lock_guard<std::mutex> mu(m_mutex_connections);
		if (!m_connections.count(receiverID)) {
			return false;
		}

		conn = m_connections[receiverID];
	}

	if (!conn) {
		return false;
	}

	if (!conn->isConnected) {
		return false;
	}

	if (::send(conn->socket, msg, packetSize, 0) == SOCKET_ERROR) {
		return false;
	}

	delete[] msg;
	return true;
}

bool Network::send(const char* message, size_t size, Connection* conn)
{
	if (!conn->isConnected) {
		return false;
	}

	// serialize the packet size and place that information at the beginning of the packet
	std::ostringstream os(std::ios::binary);
	{
		cereal::PortableBinaryOutputArchive ar(os);
		ar(size);
	}
	std::string msgSizeString = os.str();

	size_t packetSize = MSG_SIZE_STR_LEN + size;
	char* msg = SAIL_NEW char[packetSize]();

	// The message starts with a size_t stating how large the message is
	memcpy(&msg[0], msgSizeString.c_str(), MSG_SIZE_STR_LEN);

	// The rest of msg is the actual message
	memcpy(&msg[MSG_SIZE_STR_LEN], message, size);

	if (::send(conn->socket, msg, packetSize, 0) == SOCKET_ERROR) {
		delete[] msg;
		return false;
	}

	delete[] msg;
	return true;
}

void Network::setServerMetaDescription(char* desc, int descSize)
{
	memset(m_serverMetaDesc, 0, HOST_META_DESC_SIZE);
	memcpy(m_serverMetaDesc, desc, descSize);
}

bool Network::searchHostsOnLan()
{
	if (!startUDPSocket(m_udp_localbroadcastport)) {
		return false;
	}

	UDP_DATA udpdata = { 0 };
	udpdata.package.packagetype = UDP_DATA_PACKAGE_TYPE_HOSTINFO_REQUEST;
	if (::sendto(m_udp_broadcast_socket, (char*)& udpdata, sizeof(udpdata), 0, (sockaddr*)& m_udp_broadcast_address, sizeof(m_udp_broadcast_address)) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
		int err = WSAGetLastError();
		printf("Send Error: %d", err);
#endif
		return false;
	}

	return true;
}

bool Network::startUDPSocket(unsigned short port)
{
	ULONG bAllow = 1;

	/*===UDP BROADCASTER===*/
	if (m_udp_broadcast_socket > 0) {
		return true;
	}
	m_udp_broadcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_udp_broadcast_socket == INVALID_SOCKET) {
#ifdef DEBUG_NETWORK
		printf("Error creating socket with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	if (setsockopt(m_udp_broadcast_socket, SOL_SOCKET, SO_BROADCAST, (char*)& bAllow, sizeof(bAllow)) != 0) {
#ifdef DEBUG_NETWORK
		printf("Error setsockopt with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	m_udp_broadcast_address.sin_family = AF_INET;
	m_udp_broadcast_address.sin_port = htons(port);
	m_udp_broadcast_address.sin_addr.S_un.S_addr = INADDR_BROADCAST;

	/*===UDP LISTENER===*/

	if (m_udp_directMessage_socket > 0) {
		return true;
	}
	m_udp_directMessage_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_udp_directMessage_socket == INVALID_SOCKET) {
#ifdef DEBUG_NETWORK
		printf("Error creating socket with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	if (setsockopt(m_udp_directMessage_socket, SOL_SOCKET, SO_BROADCAST, (char*)& bAllow, sizeof(bAllow)) != 0) {
#ifdef DEBUG_NETWORK
		printf("Error setsockopt with error: %d\n", WSAGetLastError());
#endif
		return false;
	}
	BOOL b = TRUE;
	if (setsockopt(m_udp_directMessage_socket, SOL_SOCKET, SO_REUSEADDR, (char*)& b, sizeof(b)) != 0) {
#ifdef DEBUG_NETWORK
		printf("Error setsockopt with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	m_udp_direct_address.sin_family = AF_INET;
	m_udp_direct_address.sin_port = htons(port);
	m_udp_direct_address.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(m_udp_directMessage_socket, (sockaddr*)& m_udp_direct_address, sizeof(m_udp_direct_address)) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
		printf("Error binding socket with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	m_UDPListener = SAIL_NEW std::thread(&Network::listenForUDP, this);

	return true;
}

void Network::listenForUDP()
{
	UDP_DATA udpdata;
	sockaddr_in client = { 0 };
	int clientSize = sizeof(sockaddr_in);

	while (!m_shutdown)
	{
		memset(&udpdata, 0, sizeof(udpdata));
		int bytesRec = recvfrom(m_udp_directMessage_socket, (char*)& udpdata, sizeof(udpdata), 0, (sockaddr*)& client, &clientSize);
		if (bytesRec > 0) {
			client.sin_port = htons(m_udp_localbroadcastport); //this is might needed to do direct UDP instead of broadcast

			switch (udpdata.package.packagetype)
			{
			case UDP_DATA_PACKAGE_TYPE_HOSTINFO:
				if (m_initializedStatus) {
#ifdef DEBUG_NETWORK
					//printf("UDP_DATA_PACKAGE_TYPE_HOSTINFO: %d - %s\n", udpdata.package.packageData.hostdata.port, udpdata.package.packageData.hostdata.description);
#endif // DEBUG_NETWORK

					NetworkEvent nEvent;
					nEvent.eventType = NETWORK_EVENT_TYPE::HOST_ON_LAN_FOUND;
					nEvent.clientID = 0;
					NetworkEventData data = NetworkEventData();
					// Memcpy description before the other shit, as it writes over rawMsg.
					memcpy(data.HostFoundOnLanData.description, udpdata.package.packageData.hostdata.hostdescription, sizeof(m_serverMetaDesc));
					// Attatch hostPort and ip, then an irrelevant message as UDP only cares about ip and hostport atm.
					data.HostFoundOnLanData.hostPort = udpdata.package.packageData.hostdata.port;
					data.HostFoundOnLanData.ip_full = client.sin_addr.S_un.S_addr;
					

					nEvent.data = &data;

					addNetworkEvent(nEvent, sizeof(data));
				}
				break;
			case UDP_DATA_PACKAGE_TYPE_HOSTINFO_REQUEST:
				if (m_initializedStatus == INITIALIZED_STATUS::IS_SERVER) {
					UDP_DATA udpdata2 = { 0 };
					udpdata2.package.packagetype = UDP_DATA_PACKAGE_TYPE_HOSTINFO;
					udpdata2.package.packageData.hostdata.port = m_hostPort;
					memcpy(udpdata2.package.packageData.hostdata.hostdescription, m_serverMetaDesc, sizeof(m_serverMetaDesc));

					if (::sendto(m_udp_broadcast_socket, udpdata2.raw_data, sizeof(udpdata2), 0, (sockaddr*)& m_udp_broadcast_address, sizeof(sockaddr_in)) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
						printf("Error sendto udp with error: %d\n", WSAGetLastError());
#endif // DEBUG_NETWORK
					}
				}
				break;
			default:
#ifdef DEBUG_NETWORK
				printf("Not understanding UDP message: %s\n", udpdata.raw_data);
#endif // DEBUG_NETWORK
				break;
			}
		}
		else {
#ifdef DEBUG_NETWORK
			printf("Error recvfrom with error: %d\n", WSAGetLastError());
#endif // DEBUG_NETWORK
		}
	}
}

bool Network::udpSend(sockaddr* addr, char* msg, int msgSize)
{
	if (::sendto(m_udp_directMessage_socket, msg, msgSize, 0, addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
		printf("Error sendto udp with error: %d\n", WSAGetLastError());
#endif
		return false;
	}

	return true;
}

TCP_CONNECTION_ID Network::generateID()
{
	TCP_CONNECTION_ID id = 0;
	std::random_device rd;   // non-deterministic generator.
	std::mt19937 gen(rd());

	if (m_hostFlags & (USHORT)HostFlags::USE_RANDOM_IDS) {
		int n = sizeof(TCP_CONNECTION_ID) / sizeof(int);

		for (size_t i = 0; i < n; i++)
		{
			unsigned int r = gen();
			id |= (TCP_CONNECTION_ID)r << (i * 32);
		}
	}
	else {
		id = m_nextID++;
	}

	return id;
}

bool Network::isServer() {
	return m_initializedStatus == INITIALIZED_STATUS::IS_SERVER;
}

Network::INITIALIZED_STATUS Network::getInitializeStatus() {
	return m_initializedStatus;
}

void Network::shutdown()
{
	if (!m_initializedStatus) {
		return;
	}

	m_shutdown = true;

	if (m_initializedStatus == INITIALIZED_STATUS::IS_SERVER) {
		::shutdown(m_soc, 2);
		if (closesocket(m_soc) == SOCKET_ERROR) {
			m_soc = 0;
#ifdef DEBUG_NETWORK
			printf("Error closing m_soc\n");
#endif
		}
		else if (m_clientAcceptThread) {
			m_clientAcceptThread->join();
			delete m_clientAcceptThread;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex_connections);
		for (auto it : m_connections)
		{
			Connection* conn = it.second;
			::shutdown(conn->socket, 2);
			if (closesocket(conn->socket) == SOCKET_ERROR) {
#ifdef DEBUG_NETWORK
				printf((std::string("Error closing socket") + std::to_string(conn->id) + "\n").c_str());
#endif
			}
			else if (conn->thread) {
				conn->thread->join();
				delete conn->thread;
				delete conn;
			}
		}
	}

	if (m_udp_broadcast_socket) {
		closesocket(m_udp_broadcast_socket);
		m_udp_broadcast_socket = 0;
	}
	if (m_udp_directMessage_socket) {
		closesocket(m_udp_directMessage_socket);
		m_udp_directMessage_socket = 0;
	}

	if (m_UDPListener) {
		m_UDPListener->join();
		delete 	m_UDPListener;
		m_UDPListener = nullptr;
	}

	m_connections.clear();
	m_initializedStatus = INITIALIZED_STATUS::INITIALIZED;
}

void Network::ip_int_to_ip_string(ULONG ip, char* buffer, int buffersize)
{
	inet_ntop(AF_INET, &ip, buffer, buffersize);
}

ULONG Network::ip_string_to_ip_int(char* ip)
{
	int result = 0;
	inet_pton(AF_INET, ip, &result);
	return result;
}

void Network::addNetworkEvent(NetworkEvent n, int dataSize, const char* data) {
	std::lock_guard<std::mutex> lock(m_mutex_packages);
	// delete previous message if there is one
	if (m_awaitingMessages[m_pend].Message.rawMsg != nullptr) {
		delete[] m_awaitingMessages[m_pend].Message.rawMsg;
		m_awaitingMessages[m_pend].Message.rawMsg = nullptr;
	}

	// Copy the incoming data to a message
	
	if (n.eventType == NETWORK_EVENT_TYPE::HOST_ON_LAN_FOUND) {
		// UDP MESSAGE
		memcpy(&m_awaitingMessages[m_pend].HostFoundOnLanData, &n.data->HostFoundOnLanData, sizeof(n.data->HostFoundOnLanData));
	}
	else {
		// All other messages
		m_awaitingMessages[m_pend].Message.rawMsg = SAIL_NEW char[dataSize]();
		memcpy(m_awaitingMessages[m_pend].Message.rawMsg, data, dataSize);
		m_awaitingMessages[m_pend].Message.sizeOfMsg = dataSize;
	}



	m_awaitingEvents[m_pend].eventType = n.eventType;
	m_awaitingEvents[m_pend].clientID = n.clientID;
	m_awaitingEvents[m_pend].data = &m_awaitingMessages[m_pend];

	m_pend = (m_pend + 1) % MAX_AWAITING_PACKAGES;
	if (m_pend == m_pstart) {
		m_pstart++;
	}
}

void Network::waitForNewConnections()
{
	while (!m_shutdown)
	{
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(m_soc, (sockaddr*)& client, &clientSize);
		if (clientSocket == INVALID_SOCKET) {
			//Do something
			continue;
		}

		char host[NI_MAXHOST] = { 0 }; // Client's remote name

		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);

		if (true) {
			Connection* conn = SAIL_NEW Connection;
			conn->isConnected = true;
			conn->socket = clientSocket;
			conn->ip = host;
			conn->port = ntohs(client.sin_port);

			bool ok = false;
			do {
				conn->id = generateID();
				if (m_connections.find(conn->id) == m_connections.end()) {
					ok = true;
				}
			} while (!ok);

			conn->thread = SAIL_NEW std::thread(&Network::listen, this, conn); //Create new listening thread for the new connection
			m_connections[conn->id] = conn;
		}
	}
}

void Network::listen(const Connection* conn)
{
	NetworkEvent nEvent;
	nEvent.clientID = conn->id;
	nEvent.eventType = NETWORK_EVENT_TYPE::CONNECTION_ESTABLISHED;
	addNetworkEvent(nEvent, 0);

	bool connectionIsClosed = false;
	char incomingPackageSize[MSG_SIZE_STR_LEN];
	int bytesToReceive = 0;

	while (!connectionIsClosed && !m_shutdown) {
		ZeroMemory(incomingPackageSize, MSG_SIZE_STR_LEN);

		// Find out how large the incoming packet is
		recv(conn->socket, incomingPackageSize, MSG_SIZE_STR_LEN, 0);

		// Read how many bytes to receive
		std::istringstream is(std::string(incomingPackageSize, MSG_SIZE_STR_LEN));
		{
			cereal::PortableBinaryInputArchive ar(is);
			ar(bytesToReceive);
		}

		// Get the incoming packet and place it in a char array
		char* msg = SAIL_NEW char[bytesToReceive]();
		int bytesReceived = recv(conn->socket, msg, bytesToReceive, 0);

#ifdef DEBUG_NETWORK
		printf((std::string("bytesReceived: ") + std::to_string(bytesReceived) + "\n").c_str());
#endif // DEBUG_NETWORK		
		switch (bytesReceived)
		{
		case 0:
#ifdef DEBUG_NETWORK
			printf("Client Disconnected\n");
#else
		case SOCKET_ERROR:
#endif // DEBUG_NETWORK
			connectionIsClosed = true;
			nEvent.eventType = NETWORK_EVENT_TYPE::CONNECTION_CLOSED;
			addNetworkEvent(nEvent, 0);
			break;
#ifdef DEBUG_NETWORK
		case SOCKET_ERROR:
			printf("Error Receiving: Disconnecting client.\n");
			connectionIsClosed = true;
			nEvent.eventType = NETWORK_EVENT_TYPE::CONNECTION_CLOSED;
			addNetworkEvent(nEvent, 0);
			break;
#endif // DEBUG_NETWORK
		default:
			nEvent.eventType = NETWORK_EVENT_TYPE::MSG_RECEIVED;
			addNetworkEvent(nEvent, bytesReceived, msg);

			break;
		}

		delete[] msg;
	}
}