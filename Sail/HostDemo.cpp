//#include "Network/NetworkModule.hpp"
////#include "pch.h"
//
//static int counter = 0;
//
////PlayerJoined: Do whatever
//void PlayerJoined(ConnectionID id) {
//	printf((std::string("New player Joined with id: ") + std::to_string(id) + "\n").c_str());
//}
////PlayerDisconnected: Do whatever
//void PlayerDisconnected(ConnectionID id) {
//	printf((std::string("Player Disconnected with id: ") + std::to_string(id) + "\n").c_str());
//}
////PlayerReconnected: Do whatever
//void PlayerReconnected(ConnectionID id) {
//	printf((std::string("Player Reconnected with id: ") + std::to_string(id) + "\n").c_str());
//}
////MessageRecieved: Do whatever
//void DecodeMessage(NetworkEvent nEvent) {
//	counter++;
//	std::string s = "Client #" + std::to_string(nEvent.clientID) + " says: ";
//	s += nEvent.data->msg;
//	s += "\n";
//
//	printf(s.c_str());
//}
//
//void CALLBACK ProcessPackage(NetworkEvent nEvent) {
//
//	switch (nEvent.eventType)
//	{
//	case NETWORK_EVENT_TYPE::NETWORK_ERROR:
//		break;
//	case NETWORK_EVENT_TYPE::CLIENT_JOINED:
//		PlayerJoined(nEvent.clientID);
//		break;
//	case NETWORK_EVENT_TYPE::CLIENT_DISCONNECTED:
//		PlayerDisconnected(nEvent.clientID);
//		break;
//	case NETWORK_EVENT_TYPE::CLIENT_RECONNECTED:
//		PlayerReconnected(nEvent.clientID);
//		break;
//	case NETWORK_EVENT_TYPE::MSG_RECEIVED:
//		DecodeMessage(nEvent);
//		break;
//	default:
//		break;
//	}
//}
//
//int main() {
//	SetConsoleTitle(("This is The Host"));
//
//	Network n;
//	bool ok = n.SetupHost(54000);
//
//	char msg[] = "Server Says Hi!";
//	n.Send(msg, sizeof(msg), -1);
//
//	//Keep main from closing
//	while (ok) {
//		if (counter % 3 == 0) {
//			counter++;
//			n.Send(msg, sizeof(msg), -1);
//		}
//		if (counter < 320) {
//			n.CheckForPackages(&ProcessPackage);
//		}
//		else
//		{
//			ok = false;
//		}
//	};
//
//	std::this_thread::sleep_for(std::chrono::seconds(2));
//
//	printf("Closing Server...");
//	n.Shutdown();
//	printf(" OK");
//
//	return 0;
//}