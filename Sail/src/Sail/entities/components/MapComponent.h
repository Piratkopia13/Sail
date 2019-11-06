#pragma once

#include "Component.h"
#include <vector>
#include <queue>



enum AreaType {CORRIDOR, ROOM};

class MapComponent : public Component<MapComponent> {
public:
	MapComponent(int hostSeed) {
	}
	~MapComponent() {
	}

//#ifdef _PERFORMANCE_TEST
//	const static int xsize = 20, ysize = 20; //size of level
//	int tileArr[xsize][ysize][3]; //0 is tileID, 1 is typeID, 2 is door
//	float hallwayThreshold = 0.3f; // percentage of level that can be corridors
//	int minSplitSize = 5; //minimum size for splitting chunks
//	int minRoomSize = 1; //minimum side of a room
//	int roomMaxSize = 36;//maximum area of a room
//	int roomSplitStop = 25;//percentage to stop a room from being split into smaller ones
//	int doorModifier = 15;//percentage to spawn a door
//	int clutterModifier = 85;//percentage to add clutter
//	int seed;//seed for generation
//#else


//#ifdef _DEBUG
//	const static int xsize = 7;
//	const static int ysize = 7; //size of level
//#elif DEVELOPMENT
//	const static int xsize = 8;
//	const static int ysize = 8; //size of level
//#else 
//	const static int xsize = 10;
//	const static int ysize = 10; //size of level
//#endif
};