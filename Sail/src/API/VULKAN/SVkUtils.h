#pragma once
#include "vulkan/vulkan_core.h"
#include "Sail/api/shader/Shader.h"

#ifdef _DEBUG
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define VK_CHECK_RESULT(r) \
{ \
	std::string cmd(#r); \
	cmd = cmd.substr(0, cmd.find("(")); \
	VkResult res = r; \
	if (res != VK_SUCCESS) { \
		Logger::Error("Command " + cmd + " failed with VkResult " + SVkUtils::ErrorString(res) + " in " + std::string(__FILENAME__) + " at line " + std::to_string(__LINE__)); \
	} \
}
#else
#define VK_CHECK_RESULT(r) \
{ \
	VkResult res = r; \
	if (res != VK_SUCCESS) { \
		Logger::Error("A VK command failed with VkResult " + SVkUtils::ErrorString(res)); \
	} \
}
#endif

namespace SVkUtils {
	VkShaderStageFlags ConvertShaderBindingToStageFlags(ShaderComponent::BIND_SHADER bindShader);
	std::string ErrorString(VkResult errorCode);

	void TransitionImageLayout(const VkCommandBuffer& cmd, const VkImage& image, VkFormat format, const VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int layerCount = 1, unsigned int mipLevels = 1);
}