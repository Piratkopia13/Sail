#pragma once

// Memory leak detection for debug
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define SAIL_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define SAIL_NEW new
#endif

#define NOMINMAX // Removes min max macros which cause issues
// Exclude some less used APIs to speed up the build process on windows
#define WIN32_LEAN_AND_MEAN

// Math
// TODO: only define GLM_FORCE_DEPTH_ZERO_TO_ONE if directx or vulkan (not opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <windows.h>

#include <memory>
#include <comdef.h> 
#include <string>
#include <Memory>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>

// DirectX Toolkit includes
//#include <d3d12.h>
//#include <Keyboard.h>
//#include <Mouse.h>
//#include <GamePad.h>
