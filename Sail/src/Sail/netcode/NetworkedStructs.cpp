#include "pch.h"
#include "NetworkedStructs.h"

namespace Netcode {
	// Start at 1 so that we can use zero to tell if a variable hasn't been set yet.
	std::atomic<ComponentID> gNetworkIDCounter = 1;
}
