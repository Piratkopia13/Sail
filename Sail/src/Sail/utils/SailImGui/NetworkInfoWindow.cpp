#include "pch.h"
#include "NetworkInfoWindow.h"
#include "imgui.h"
#include "Network/NWrapperSingleton.h"
#include "Network/NWrapperHost.h"

NetworkInfoWindow::NetworkInfoWindow(bool showWindow) {
}

NetworkInfoWindow::~NetworkInfoWindow() {
}

void NetworkInfoWindow::renderWindow() {
	NWrapperSingleton &net = NWrapperSingleton::getInstance();
	
	ImGui::Begin("Network Wrapper Info");
	ImGui::Columns(2);
	ImGui::Text("IsHost:");
	ImGui::NextColumn();
	ImGui::Text(net.isHost() ? "TRUE" : "FALSE");
	ImGui::NextColumn();
	ImGui::Columns(1);
	if (ImGui::CollapsingHeader("Player List")) {
		ImGui::Columns(3);
		ImGui::Text("Player ID");
		ImGui::NextColumn();

		ImGui::Text("Name");
		ImGui::NextColumn();
		ImGui::Text("Team");
		ImGui::NextColumn();
		ImGui::Separator();
		for (auto p : net.getPlayers()) {
			ImGui::Text(std::to_string(p.id).c_str());
			ImGui::NextColumn();
			ImGui::Text(p.name.c_str());
			ImGui::NextColumn();
			ImGui::Text(std::to_string((int)p.team).c_str());
			ImGui::NextColumn();
		}
	}

#ifdef DEVELOPMENT
	if (net.isHost()) {
		NWrapperHost* nw_host = static_cast<NWrapperHost*>(net.getNetworkWrapper());
		ImGui::Columns(1);
		if (ImGui::CollapsingHeader("Host Only Data")) {
			ImGui::Columns(2);

			ImGui::Text("Lobby Name:");
			ImGui::NextColumn();
			ImGui::Text(nw_host->getLobbyName().c_str());
			ImGui::NextColumn();

			ImGui::Text("Server Description:");
			ImGui::NextColumn();
			ImGui::Text(nw_host->getServerDescription().c_str());
			ImGui::NextColumn();

			ImGui::Columns(1);
			if (ImGui::CollapsingHeader("TCP Connection IDs")) {
				ImGui::Columns(2);
				ImGui::Text("TCP ID");
				ImGui::NextColumn();
				ImGui::Text("Player ID");
				ImGui::NextColumn();
				ImGui::Separator();

				for (auto p : nw_host->getConnectionMap()) {
					ImGui::Text(std::to_string(p.first).c_str());
					ImGui::NextColumn();
					ImGui::Text(std::to_string(p.second).c_str());
					ImGui::NextColumn();
				}
			}

		}
	}
#endif // DEVELOPMENT



	ImGui::End();
}
