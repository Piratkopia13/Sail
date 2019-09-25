#pragma once

#include "Component.h"
#include <vector>


class MapComponent : public Component<MapComponent> {
public:
	MapComponent() {

	}
	~MapComponent() {}
	std::vector<int> m_tiles;
	int tileArr[10][10];
	int xsize=10, ysize=10;
private:
};