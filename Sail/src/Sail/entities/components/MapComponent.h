#pragma once

#include "Component.h"
#include <vector>


class MapComponent : public Component<MapComponent> {
public:
	MapComponent() {
		for (int i = 0; i < xsize; i++) {
			for (int j = 0; j < ysize; j++) {
				tileArr[i][j] = -1;
			}
		}
	}
	~MapComponent() {}
	int tileArr[30][30];
	int xsize=30, ysize=30;
private:
};