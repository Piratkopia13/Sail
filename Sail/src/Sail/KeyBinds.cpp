#include "pch.h"
#include "KeyBinds.h"
#include "Sail/KeyCodes.h"
#include "Sail/MouseButtonCodes.h"

// Defines the variables
namespace KeyBinds {
	// GameState + Player controls
	int addLight;
	int showBoundingBoxes;
	int hideBoundingBoxes;
	int testRayIntersection;
	int toggleAIFollowing;
	int setDirectionalLight;
	int toggleConsole;
	int reloadShader;
	int removeOldestLight;
	int sprint;
	int moveForward;
	int moveBackward;
	int moveRight;
	int moveLeft;
	int moveUp;
	int moveDown;
	int lightCandle1;
	int lightCandle2;
	int putDownCandle;
	int disableCursor;
	int shoot;
	int showInGameMenu;
	int toggleSun;

	// Application
	int alt;
	int f4;

	// Audio
	int _0;
	int _1;
	int _2;
	int _3;
	int _9;

	// StateStack
	int toggleImGui;

	// LobbyState
	int sendMessage;

	// DX12RaytracingRenderer
	int reloadDXRShader;
}

void KeyBinds::init() {
	// GameState
	addLight = SAIL_KEY_E;
	showBoundingBoxes = SAIL_KEY_B;
	hideBoundingBoxes = SAIL_KEY_N;
	testRayIntersection = SAIL_KEY_O;
	toggleAIFollowing = SAIL_KEY_H;
	setDirectionalLight = SAIL_KEY_G;
	toggleConsole = SAIL_KEY_OEM_5;
	reloadShader = SAIL_KEY_R;
	removeOldestLight = SAIL_KEY_X;
	sprint = SAIL_KEY_SHIFT;
	moveForward = SAIL_KEY_W;
	moveBackward = SAIL_KEY_S;
	moveRight = SAIL_KEY_D;
	moveLeft = SAIL_KEY_A;
	moveUp = SAIL_KEY_SPACE;
	moveDown = SAIL_KEY_CONTROL;
	lightCandle1 = SAIL_KEY_Z;
	lightCandle2 = SAIL_KEY_V;
	putDownCandle = SAIL_KEY_F;
	disableCursor = SAIL_MOUSE_RIGHT_BUTTON;
	shoot = SAIL_MOUSE_LEFT_BUTTON;
	showInGameMenu = SAIL_KEY_ESCAPE;
	toggleSun = SAIL_KEY_P;

	// Application
	alt = SAIL_KEY_MENU;		// Did not know what to call these binds
	f4 = SAIL_KEY_F4;

	// Audio
	_0 = SAIL_KEY_0;		// Did not know what to call these binds
	_1 = SAIL_KEY_1;
	_2 = SAIL_KEY_2;
	_3 = SAIL_KEY_3;
	_9 = SAIL_KEY_9;

	// StateStack
	toggleImGui = SAIL_KEY_F10;

	// LobbyState
	sendMessage = SAIL_KEY_RETURN;

	// DX12RaytracingRenderer
	reloadDXRShader = SAIL_KEY_F5;

}