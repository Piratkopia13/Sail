#include "NetworkModule.hpp"
//
//static BOOL bOptVal = TRUE;
//static int bOptLen = sizeof(bool);

Network::Network() {}

Network::~Network() {}

void Network::CheckForPackages(void(*m_callbackfunction)(NetworkEvent))
{

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

bool Network::SetupHost(unsigned short port)
{
	if (m_isInitialized)
		return false;

	m_awaitingMessages = new MessageData[MAX_AWAITING_PACKAGES];
	m_awaitingEvents = new NetworkEvent[MAX_AWAITING_PACKAGES];

	WSADATA data;
	WORD version = MAKEWORD(2, 2); // use version winsock 2.2
	int status = WSAStartup(version, &data);
	if (status != 0) {
		printf("Error starting WSA\n");
		return false;
	}
	m_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //IPPROTO_TCP
	if (m_soc == INVALID_SOCKET) {
		printf("Error creating socket\n");
	}
	m_myAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	m_myAddr.sin_family = AF_INET;
	m_myAddr.sin_port = htons(port);

	if (bind(m_soc, (sockaddr*)& m_myAddr, sizeof(m_myAddr)) == SOCKET_ERROR) {
		printf("Error binding socket\n");
		return false;
	}

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(bool);

	int iOptVal = 0;
	int iOptLen = sizeof(int);

	setsockopt(m_soc, IPPROTO_TCP, TCP_NODELAY, (char*)& bOptVal, bOptLen);//Prepare the socket to listen
	setsockopt(m_soc, IPPROTO_TCP, SO_SNDBUF, (char*)& iOptVal, iOptLen);//Prepare the socket to listen

	listen(m_soc, SOMAXCONN);

	m_isInitialized = true;
	m_isServer = true;

	//Start a new thread that will wait for new connections
	m_clientAcceptThread = new std::thread(&Network::WaitForNewConnections, this);

	return true;
}

bool Network::SetupClient(const char* IP_adress, unsigned short hostport)
{
	if (m_isInitialized)
		return false;

	m_awaitingMessages = new MessageData[MAX_AWAITING_PACKAGES];
	m_awaitingEvents = new NetworkEvent[MAX_AWAITING_PACKAGES];

	WSADATA data;
	WORD version = MAKEWORD(2, 2); // use version winsock 2.2
	int status = WSAStartup(version, &data);
	if (status != 0) {
		printf("Error starting WSA\n");
		return false;
	}

	m_soc = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
	if (m_soc == INVALID_SOCKET) {
		printf("Error creating socket\n");
		return false;
	}

	//m_myAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	m_myAddr.sin_family = AF_INET;
	m_myAddr.sin_port = htons(hostport);
	inet_pton(AF_INET, IP_adress, &m_myAddr.sin_addr);

	//connect needs error checking!
	int conres = connect(m_soc, (sockaddr*)& m_myAddr, sizeof(m_myAddr));
	if (conres == SOCKET_ERROR) {
		printf("Error connecting to host\n");
		return false;
	}

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(bool);

	int iOptVal = 0;
	int iOptLen = sizeof(int);

	setsockopt(m_soc, IPPROTO_TCP, TCP_NODELAY, (char*)& bOptVal, bOptLen);//Prepare the socket to listen
	setsockopt(m_soc, IPPROTO_TCP, SO_SNDBUF, (char*)& iOptVal, iOptLen);//Prepare the socket to listen


	Connection conn;
	conn.isConnected = true;
	conn.socket = m_soc;
	conn.ip = "";
	conn.port = ntohs(m_myAddr.sin_port);
	conn.id = 0;
	conn.thread = new std::thread(&Network::Listen, this, conn); //Create new listening thread listening for the host
	m_connections.push_back(conn);

	m_isInitialized = true;
	m_isServer = false;

	return true;
}

bool Network::Send(const char* message, size_t size, ConnectionID receiverID)
{
	if (receiverID == -1 && m_isServer) {
		int n = 0;
		{
			std::lock_guard<std::mutex> mu(m_mutex_connections);
			n = m_connections.size();
		}
		int success = 0;
		for (int i = 0; i < n; i++)
		{
			if (Send(message, size, i))
				success++;
		}
		//printf((std::string("Broadcasting to ") + std::to_string(success) + "/" + std::to_string(n) + " Clients: " + std::string(message) + "\n").c_str());

		return true;
	}

	char msg[MAX_PACKAGE_SIZE] = { 0 };
	memcpy(msg, message, size);

	Connection conn;
	{
		std::lock_guard<std::mutex> mu(m_mutex_connections);
		if (receiverID > m_connections.size())
			return false;

		conn = m_connections[receiverID];
	}

	if (!conn.isConnected)
		return false;

	if (send(conn.socket, msg, MAX_PACKAGE_SIZE, 0) == SOCKET_ERROR)
		return false;

	return true;
}

bool Network::Send(const char* message, size_t size, Connection conn)
{
	if (!conn.isConnected)
		return false;

	if (send(conn.socket, message, size, 0) == SOCKET_ERROR)
		return false;

	return true;
}

void Network::Shutdown()
{
	m_shutdown = true;

	if (m_isServer) {
		::shutdown(m_soc, 2);
		if (closesocket(m_soc) == SOCKET_ERROR) {
			printf("Error closing m_soc\n");
		}
		else if (m_clientAcceptThread) {
			m_clientAcceptThread->join();
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex_connections);
		for (auto conn : m_connections)
		{
			::shutdown(conn.socket, 2);
			if (closesocket(conn.socket) == SOCKET_ERROR) {
				printf((std::string("Error closing socket") + std::to_string(conn.id) + "\n").c_str());
			}
			else if (conn.thread) {
				conn.thread->join();
			}
		}
	}

	delete[] m_awaitingEvents;
	delete[] m_awaitingMessages;
}

void Network::AddNetworkEvent(NetworkEvent n, int dataSize)
{
	std::lock_guard<std::mutex> lock(m_mutex_packages);

	memcpy(m_awaitingMessages[m_pend].msg, n.data->msg, dataSize);
	m_awaitingEvents[m_pend].eventType = n.eventType;
	m_awaitingEvents[m_pend].clientID = n.clientID;
	m_awaitingEvents[m_pend].data = &m_awaitingMessages[m_pend];

	m_pend = (m_pend + 1) % MAX_AWAITING_PACKAGES;
	if (m_pend == m_pstart)
		m_pstart++;
}

void Network::WaitForNewConnections()
{
	while (!m_shutdown)
	{
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(m_soc, (sockaddr*)& client, &clientSize);
		if (clientSocket == INVALID_SOCKET) {
			//Do something
			return;
		}

		char host[NI_MAXHOST] = { 0 }; // Client's remote name


		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);

#ifdef DEBUG_NETWORK
		std::string s = host;
		s += " connected on port " + std::to_string(ntohs(client.sin_port)) + "\n";
		printf(s.c_str());
#endif // DEBUG_NETWORK

		Connection conn;
		conn.isConnected = true;
		conn.socket = clientSocket;
		conn.ip = host;
		conn.port = ntohs(client.sin_port);

		{
			//Critical region for m_connections
			std::lock_guard<std::mutex> lock(m_mutex_connections);
			conn.id = m_connections.size();
			conn.thread = new std::thread(&Network::Listen, this, conn); //Create new listening thread for the new connection
			m_connections.push_back(conn);
		}
	}
}

void Network::Listen(const Connection conn)
{

	NetworkEvent nEvent;
	nEvent.clientID = conn.id;
	nEvent.eventType = NETWORK_EVENT_TYPE::CLIENT_JOINED;
	AddNetworkEvent(nEvent, 0);

	bool connectionIsClosed = false;
	char msg[MAX_PACKAGE_SIZE];

	while (!connectionIsClosed && !m_shutdown) {
		ZeroMemory(msg, sizeof(msg));
		int bytesReceived = recv(conn.socket, msg, MAX_PACKAGE_SIZE, 0);
#ifdef DEBUG_NETWORK
		printf((std::string("bytesReceived: ") + std::to_string(bytesReceived) + "\n").c_str());
#endif // DEBUG_NETWORK		
		switch (bytesReceived)
		{
		case 0:
#ifdef DEBUG_NETWORK
			printf("Client Disconnected\n");
#endif // DEBUG_NETWORK
			connectionIsClosed = true;
			nEvent.eventType = NETWORK_EVENT_TYPE::CLIENT_DISCONNECTED;
			AddNetworkEvent(nEvent, 0);
			break;
		case SOCKET_ERROR:
#ifdef DEBUG_NETWORK
			printf("Error Receiving: Disconnecting client.\n");
#endif // DEBUG_NETWORK
			connectionIsClosed = true;
			nEvent.eventType = NETWORK_EVENT_TYPE::CLIENT_DISCONNECTED;
			AddNetworkEvent(nEvent, 0);
			break;
		default:

			nEvent.data = reinterpret_cast<MessageData*>(msg);
			nEvent.eventType = NETWORK_EVENT_TYPE::MSG_RECEIVED;

			AddNetworkEvent(nEvent, bytesReceived);

			break;
		}
	}
}
