#pragma once

#include "../BaseComponentSystem.h"
#include <vector>
#include <string>


class NetworkClientSystem : public BaseComponentSystem {
public:
	NetworkClientSystem();
	virtual ~NetworkClientSystem();

	void update(float dt);


private:
};



