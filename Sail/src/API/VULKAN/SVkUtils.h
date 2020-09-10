#pragma once
#include "vulkan/vulkan_core.h"
#include "Sail/api/shader/Shader.h"

namespace SVkUtils {
	VkShaderStageFlags ConvertShaderBindingToStageFlags(ShaderComponent::BIND_SHADER bindShader);
}