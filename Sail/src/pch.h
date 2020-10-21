#pragma once

#include "Sail/debug/Instrumentor.h"

#ifdef _WIN32
	#ifndef NOMINMAX
		#define NOMINMAX // Removes min max macros which cause issues
	#endif 
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN // Exclude some less used APIs to speed up the build process on windows
	#endif
	#include <Windows.h>

	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef _SAIL_VK
#define VK_NO_PROTOTYPES // Volk handles prototypes
#define VK_ENABLE_BETA_EXTENSIONS // Raytracing is currently considered a beta feature
#endif

// Math
// Only define GLM_FORCE_DEPTH_ZERO_TO_ONE if directx or vulkan (not opengl)
#if defined(_SAIL_DX11) || defined(_SAIL_DX12) || defined(_SAIL_VK)
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <comdef.h>
#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>
#include <random>
#include <fstream>

// Memory tracking
//void* operator new(size_t size, char* file, int line, char* function);

// Memory leak detection for debug
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define SAIL_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#define SAIL_NEW new (__FILE__, __LINE__, __FUNCTION__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define SAIL_NEW new
#endif