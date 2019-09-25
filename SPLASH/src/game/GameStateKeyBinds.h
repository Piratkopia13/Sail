#pragma once
#include "Sail/KeyCodes.h"

struct GameStateKeyBinds {
	int addLight				= SAIL_KEY_E;
	int showBoundingBoxes		= SAIL_KEY_1;
	int hideBoundingBoxes		= SAIL_KEY_2;
	int testRayIntersection		= SAIL_KEY_O;
	int toggleAIFollowing		= SAIL_KEY_H;
	int setDirectionalLight		= SAIL_KEY_G;
	int toggleConsole			= SAIL_KEY_OEM_5;
	int reloadShader			= SAIL_KEY_R;
	int removeOldestLight		= SAIL_KEY_X;
	int sprint					= SAIL_KEY_SHIFT;
	int moveForward				= SAIL_KEY_W;
	int moveBackward			= SAIL_KEY_S;
	int moveRight				= SAIL_KEY_D;
	int moveLeft				= SAIL_KEY_A;
	int jump					= SAIL_KEY_SPACE;
	int lightCandle1			= SAIL_KEY_Z;
	int lightCandle2			= SAIL_KEY_V;
};