#pragma once

namespace KeyBinds {
	
	// Initializes the key codes
	void init();

	
	// GameState + Player controls
	extern int addLight;
	extern int showBoundingBoxes;
	extern int hideBoundingBoxes;
	extern int testRayIntersection;
	extern int toggleAIFollowing;
	extern int setDirectionalLight;
	extern int toggleConsole;
	extern int reloadShader;
	extern int removeOldestLight;
	extern int sprint;
	extern int moveForward;
	extern int moveBackward;
	extern int moveRight;
	extern int moveLeft;
	extern int moveUp;
	extern int moveDown;
	extern int lightCandle1;
	extern int lightCandle2;

	// Application
	extern int alt;
	extern int f4;

	// Audio
	extern int _0;
	extern int _1;
	extern int _2;
	extern int _3;
	extern int _9;

	// StateStack
	extern int toggleImGui;

	// LobbyState
	extern int sendMessage;
};