#pragma once

// Disable "Variable uninitialized" warning
// 218 of these in the fbx sdk
#pragma warning(disable:26495)

// TODO: only define GLM_FORCE_DEPTH_ZERO_TO_ONE if directx or vulkan (not opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"
#include "Sail/graphics/material/TexturesMaterial.h"
#include "Sail/graphics/shader/material/PhongMaterialShader.h"
#include "Sail/graphics/shader/material/PBRMaterialShader.h"
#include "Sail/graphics/shader/basic/CubemapShader.h"
#include "Sail/graphics/shader/compute/GenerateMipsComputeShader.h"
#include "Sail/graphics/geometry/factory/CubeModel.h"
#include "Sail/graphics/geometry/factory/ConeModel.h"
#include "Sail/graphics/geometry/factory/PlaneModel.h"
#include "Sail/states/StateStack.h"
#include "Sail/events/Events.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/Components.h"
#include "Sail/graphics/Scene.h"
#include "Sail/graphics/geometry/Transform.h"
