#pragma once

#include "Component.h"
#include <vector>
struct rect {
	int posx;
	int posy;
	int sizex;
	int sizey;
};
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
	float hallwayThreshold = 0.3f;
	std::queue<rect> chunks, blocks, hallways, rooms;
private:
};