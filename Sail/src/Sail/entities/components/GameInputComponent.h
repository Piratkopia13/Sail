#pragma once
#include <vector>
#include <bitset>
#include "Component.h"
#include "..//..//KeyCodes.h"

class GameInputComponent final : public Component<GameInputComponent> {
public:
	GameInputComponent();
	~GameInputComponent();

private:
	struct BoundKey {
		int keyIndex;

	};
	std::vector<
};
