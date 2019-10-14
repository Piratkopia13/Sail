#pragma once

#include "Component.h"
#include <vector>
#include <queue>

struct rect {
	int posx;
	int posy;
	int sizex;
	int sizey;
	int doors = 0;
};
class MapComponent : public Component<MapComponent> {
public:
	MapComponent() {
		for (int i = 0; i < xsize; i++) {
			for (int j = 0; j < ysize; j++) {
				tileArr[i][j][0] = 0;
				tileArr[i][j][1] = -1;
				tileArr[i][j][2] = 0;
			}
		}
	}
	~MapComponent() {
		while(chunks.size()>0){
			chunks.pop();
		}
		while (blocks.size() > 0) {
			blocks.pop();
		}

		while (hallways.size() > 0) {
			hallways.pop();
		}

		while (rooms.size() > 0) {
			rooms.pop();
		}

		while (matched.size() > 0) {
			matched.pop();
		}

	}
#ifdef _PERFORMANCE_TEST
	const static int xsize = 50, ysize = 50; //size of level
	int tileArr[xsize][ysize][3]; //0 is tileID, 1 is typeID, 2 is door
	float hallwayThreshold = 0.3f; // percentage of level that can be corridors
	int minSplitSize = 5; //minimum size for splitting chunks
	int minRoomSize = 1; //minimum side of a room
	int roomMaxSize = 36;//maximum area of a room
	int roomSplitStop = 25;//percentage to stop a room from being split into smaller ones
	int doorModifier = 15;//percentage to spawn a door
	int seed = 2;//seed for generation
#else
#ifdef _DEBUG
	const static int xsize = 7, ysize = 7; //size of level
#else
	const static int xsize = 7, ysize = 7; //size of level
#endif
	int tileArr[xsize][ysize][3]; //0 is tileID, 1 is typeID, 2 is door
	float hallwayThreshold = 0.3f; // percentage of level that can be corridors
	int minSplitSize = 5; //minimum size for splitting chunks
	int minRoomSize = 1; //minimum side of a room
	int roomMaxSize = 36;//maximum area of a room
	int roomSplitStop = 25;//percentage to stop a room from being split into smaller ones
	int doorModifier = 15;//percentage to spawn a door
	int seed = 2;//seed for generation
#endif
	int totalArea = xsize * ysize;
	int numberOfRooms = 1;
	int players;
	std::vector<int> spawnRooms;
	std::queue<rect> chunks, blocks, hallways, rooms, matched;
private:
};