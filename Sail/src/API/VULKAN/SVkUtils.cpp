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

std::string SVkUtils::ErrorString(VkResult errorCode) {
	switch (errorCode) {
#define STR(r) case VK_ ##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}
