#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#include <vector>
#include <string>
//#include <iostream>
#include <thread>
#include <mutex>

#define MAX_PACKAGE_SIZE 64
#define MAX_AWAITING_PACKAGES 1000


struct Connection
{
	std::string ip, port;
	int id;
	bool isConnected;
	SOCKET socket;
	std::thread* thread;//The thread used to listen for messages
	Connection() {
		id = -2;
		isConnected = false;
		socket = NULL;
		ip = port = "";
		thread = nullptr;
	}
};

struct Package {
	int senderId;
	char msg[MAX_PACKAGE_SIZE];
};

/*
	This function needs to be implemented by the application and passed to SetupHost or SetupClient.
*/
//void CALLBACK ProcessPackage(Package p);

class Network
{
public:
	Network();
	~Network();

	void CheckForPackages(void (*m_callbackfunction)(Package));

	/*
		Call SetupHost() to initialize a host socket. Dont call this and SetupHost() in the same application.
	*/
	bool SetupHost(unsigned short port);
	/*
		Call SetupClient() to initialize a client socket. Dont call this and SetupHost() in the same application.
	*/
	bool SetupClient(const char* host_ip, unsigned short hostport);
	/*
		Send a message to connection "receiverID".
		If SetupClient has been called, any number passed to receiverID will all send to the connected host.
		If SetupHost has been called, passing -1 to receiverID will send the message to all connected clients.

		Return true if message could be sent to all receivers.
	*/
	bool Send(const char* message, size_t size, int receiverID = 0);
	bool Send(const char* message, size_t size, Connection conn);
	
private:
	void (*m_callbackfunction)(Package); //Function pointer to the ProcessPackages function

	SOCKET m_soc = 0;
	sockaddr_in m_myAddr = {};
	std::thread* m_clientAcceptThread = nullptr;

	bool m_isServer = false;
	bool m_isInitialized = false;
	/*Do not access m_connections without mutex lock*/
	std::vector<Connection> m_connections;
	std::mutex m_mutex_connections;

	Package* m_awaitingPackages;
	int m_pstart = 0, m_pend = 0;
	std::mutex m_mutex_packages;
	//std::mutex m_mutex_pend;


	/*
		Only used by the server. This function is called in a new thread and waits for new incomming connection requests.
		Accepted connections are stored in m_connections. A new thread is created for each connection directly Listen() in order to listen for incomming messages from that connection;
	*/
	void WaitForNewConnections();

	/*
		This function is called by multiple threads. Listen for incomming messages from one specific connection.
		If this application is a client, only one thread will call Listen() which will listen to the host.
		If this application is a host, this function will be called by one thread for each client connected to the host.

		Host connection requests is handled in WaitForNewConnections()
	*/
	void Listen(const Connection conn);//Rename this function
};