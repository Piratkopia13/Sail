#pragma once

#include "NetworkSystem.h"

class  NetworkHostSystem final : public NetworkSystem {
public:
	NetworkHostSystem();
	~NetworkHostSystem();

	void update(float dt) override;
	bool onSerializedPackageRecieved(NetworkSerializedPackageEvent& event) override;

private:

	void sendPlayersTranslationToAllClients();

};