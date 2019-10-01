#pragma once

#include "Component.h"
#include <vector>
struct rect {
	int posx;
	int posy;
	int sizex;
	int sizey;
	int ns=0;
};
class MapComponent : public Component<MapComponent> {
public:
	MapComponent() {
		for (int i = 0; i < xsize; i++) {
			for (int j = 0; j < ysize; j++) {
				tileArr[i][j][0] = 0;
				tileArr[i][j][1] = -1;
			}
		}
	}
	~MapComponent() {}
	const static int xsize=30, ysize=30;
	int tileArr[xsize][ysize][2]; //0 is tileID, 1 is typeID
	float hallwayThreshold = 0.3f;
	int minSplitSize = 5;
	int minRoomSize = 1;
	int roomSplitStop = 3;
	int seed = 2;
	int totalArea = xsize * ysize;
	std::queue<rect> chunks, blocks, hallways, rooms, matched;
private:
};