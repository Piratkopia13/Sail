#pragma once

// Settings
//#define _SAIL_BREAK_ON_WARNING
#define _SAIL_BREAK_ON_ERROR

// Disable "Variable uninitialized" warning
// 218 of these in the fbx sdk
#pragma warning(disable:26495)

#define NOMINMAX // Removes min max macros which cause issues

// TODO: only define GLM_FORCE_DEPTH_ZERO_TO_ONE if directx or vulkan (not opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Sail/Application.h"
#include "Sail/Utils/Utils.h"
#include "Sail/graphics/text/Text.h"
#include "Sail/graphics/text/SailFont.h"
#include "Sail/graphics/camera/PerspectiveCamera.h"
#include "Sail/graphics/camera/OrthographicCamera.h"
#include "Sail/graphics/camera/FlyingCameraController.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/light/LightSetup.h"
//#include "Sail/graphics/shader/basic/SimpleColorShader.h"
//#include "Sail/graphics/shader/basic/SimpleTextureShader.h"
//#include "Sail/graphics/shader/basic/DirectionalLightShader.h"
//#include "Sail/graphics/shader/basic/CubeMapShader.h"
//#include "Sail/graphics/shader/basic/DepthShader.h"
#include "Sail/graphics/shader/material/MaterialShader.h"
//#include "Sail/graphics/shader/instanced/ParticleShader.h"
//#include "Sail/graphics/shader/deferred/DynBlockDeferredInstancedGeometryShader.h"
//#include "Sail/graphics/shader/deferred/DeferredInstancedGeometryShader.h"
//#include "Sail/graphics/shader/deferred/DeferredPointLightShader.h"
//#include "Sail/graphics/shader/deferred/DeferredDirectionalLightShader.h"
//#include "Sail/graphics/shader/deferred/DeferredGeometryShader.h"
#include "Sail/graphics/geometry/factory/CubeModel.h"
#include "Sail/graphics/geometry/factory/ConeModel.h"
#include "Sail/graphics/geometry/factory/PlaneModel.h"
//#include "Sail/graphics/RenderableTexture.h"
//#include "Sail/graphics/Skybox.h"
#include "Sail/graphics/renderer/DeferredRenderer.h"
#include "Sail/graphics/renderer/ForwardRenderer.h"
//#include "Sail/graphics/ParticleEmitter.h"
//#include "Sail/graphics/postprocessing/PostProcessPass.h"
//#include "Sail/resources/audio/SoundManager.h"
#include "Sail/states/StateStack.h"
#include "Sail/events/Events.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/Components.h"
#include "Sail/graphics/Scene.h"
#include "Sail/graphics/geometry/Transform.h"
