#pragma once
#include "Sail/api/GraphicsAPI.h"
#include <vulkan/vulkan.h>
#include <optional>

class VkAPI : public GraphicsAPI {
public:
	VkAPI();
	~VkAPI();

	bool init(Window* window) override;
	void clear(const glm::vec4& color) override;
	void setDepthMask(DepthMask setting) override;
	void setFaceCulling(Culling setting) override;
	void setBlending(Blending setting) override;
	void present(bool vsync = false) override;
	unsigned int getMemoryUsage() const override;
	unsigned int getMemoryBudget() const override;
	bool onResize(WindowResizeEvent& event) override;

private:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;

		bool isComplete() {
			return graphicsFamily.has_value();
		}
	};

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions() const;
	
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	void setupDebugMessenger();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice;

	const uint32_t m_WIDTH = 800;
	const uint32_t m_HEIGHT = 600;

	const std::vector<const char*> m_validationLayers;

#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif

};