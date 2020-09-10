#include "pch.h"
#include "SVkUtils.h"

VkShaderStageFlags SVkUtils::ConvertShaderBindingToStageFlags(ShaderComponent::BIND_SHADER bindShader) {
	VkShaderStageFlags stageFlags{};
	stageFlags |= (bindShader & ShaderComponent::VS) ? VK_SHADER_STAGE_VERTEX_BIT : 0;
	stageFlags |= (bindShader & ShaderComponent::PS) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
	stageFlags |= (bindShader & ShaderComponent::HS) ? VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT : 0;
	stageFlags |= (bindShader & ShaderComponent::DS) ? VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : 0;
	stageFlags |= (bindShader & ShaderComponent::GS) ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
	stageFlags |= (bindShader & ShaderComponent::CS) ? VK_SHADER_STAGE_COMPUTE_BIT: 0;
	return stageFlags;
}
