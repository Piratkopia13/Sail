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

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif

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