#pragma once
#include "Sail/api/GraphicsAPI.h"
#include <vulkan/vulkan.h>
#include <optional>

class Win32Window;

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
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions() const;
	
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	void setupDebugMessenger();

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device) const;
	bool checkDeviceExtensionSupport(const VkPhysicalDevice& device) const;
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device) const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Win32Window* window) const;

private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapChain;

	// Queues
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	const std::vector<const char*> m_validationLayers;
	const std::vector<const char*> m_deviceExtensions;

#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif

};