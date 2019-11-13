#include "pch.h"
#include "WaitingForPlayersWindow.h"
#include "imgui.h"
#include "Network/NWrapperSingleton.h"

WaitingForPlayersWindow::WaitingForPlayersWindow(bool showWindow) {
}

WaitingForPlayersWindow::~WaitingForPlayersWindow() {
}

void WaitingForPlayersWindow::renderWindow() {

	ImGui::Begin("Waiting For Players");
	ImGui::Columns(2);
	for (auto p : NWrapperSingleton::getInstance().getPlayers()) {
		if (p.lastStateStatus.status < 1) {
			ImGui::Text(p.name.c_str());
		}
	}
	
	ImGui::End();

}
