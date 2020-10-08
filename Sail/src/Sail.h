#pragma once

// Disable "Variable uninitialized" warning
// 218 of these in the fbx sdk
#pragma warning(disable:26495)

// Only define GLM_FORCE_DEPTH_ZERO_TO_ONE if directx or vulkan (not opengl)
#if defined(_SAIL_DX11) || defined(_SAIL_DX12) || defined(_SAIL_VK)
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>

#include "Sail/debug/Instrumentor.h"

#include "sail/KeyCodes.h"
#include "sail/MouseButtonCodes.h"
#include "Sail/Application.h"
#include "Sail/Utils/Utils.h"
#include "Sail/graphics/text/Text.h"
#include "Sail/graphics/text/SailFont.h"
#include "Sail/graphics/camera/PerspectiveCamera.h"
#include "Sail/graphics/camera/OrthographicCamera.h"
#include "Sail/graphics/camera/FlyingCameraController.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"
#include "Sail/graphics/material/TexturesMaterial.h"
#include "Sail/graphics/material/OutlineMaterial.h"
#include "Sail/graphics/geometry/factory/Cube.h"
#include "Sail/graphics/geometry/factory/Cone.h"
#include "Sail/graphics/geometry/factory/Plane.h"
#include "Sail/states/StateStack.h"
#include "Sail/events/Events.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/Components.h"
#include "Sail/graphics/Scene.h"
#include "Sail/graphics/Environment.h"
#include "Sail/graphics/geometry/Transform.h"
