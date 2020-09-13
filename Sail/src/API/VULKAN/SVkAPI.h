#pragma once
#include "Sail/api/GraphicsAPI.h"
#include <vulkan/vulkan.h>
#include <optional>
#include "vk_mem_alloc.h"
#include "Sail/Application.h"

class Win32Window;

class SVkAPI : public GraphicsAPI {
public:
	static const int MAX_FRAMES_IN_FLIGHT;

	struct Command {
		std::vector<VkCommandBuffer> buffers;
	};

	// Memory allocation variants
	struct BufferAllocation {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		
		void destroy() {
			if (buffer != VK_NULL_HANDLE) {
				vmaDestroyBuffer(Application::getInstance()->getAPI<SVkAPI>()->getVmaAllocator(), buffer, allocation);
			}
			buffer = VK_NULL_HANDLE;
			allocation = VK_NULL_HANDLE;
		}
		~BufferAllocation() {
			destroy();
		}
	};
	struct ImageAllocation {
		VkImage image = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;

		void destroy() {
			if (image != VK_NULL_HANDLE) {
				vmaDestroyImage(Application::getInstance()->getAPI<SVkAPI>()->getVmaAllocator(), image, allocation);
			}
			image = VK_NULL_HANDLE;
			allocation = VK_NULL_HANDLE;
		}
		~ImageAllocation() {
			destroy();
		}
	};

public:
	SVkAPI();
	~SVkAPI();

	bool init(Window* window) override;
	void clear(const glm::vec4& color) override;
	void setDepthMask(DepthMask setting) override;
	void setFaceCulling(Culling setting) override;
	void setBlending(Blending setting) override;
	void waitForGPU() override;
	
	uint32_t beginPresent(); // Returns the swap image index to use this frame
	void present(bool vsync = false) override;
	
	unsigned int getMemoryUsage() const override;
	unsigned int getMemoryBudget() const override;
	bool onResize(WindowResizeEvent& event) override;

	const VkDevice& getDevice() const;
	const VkPhysicalDevice& getPhysicalDevice() const;
	const VkViewport& getViewport() const;
	const VkRect2D& getScissorRect() const;
	const VkRenderPass& getRenderPass() const;
	uint32_t getSwapImageIndex() const; // It is only valid to call this between beginPresent() and present(), otherwise it will return -1
	size_t getNumSwapChainImages() const;
	VkRenderPassBeginInfo getRenderPassInfo() const; // TODO: maybe renderers should handle their own render passes?
	const VkDescriptorPool& getDescriptorPool() const;
	const VmaAllocator& getVmaAllocator() const;
	const uint32_t* getGraphicsAndCopyQueueFamilyIndices() const;
	
	void initCommand(Command& command) const;
	
	// Schedules copy queue commands to run at the latest during the next call to present()
	void scheduleOnCopyQueue(std::function<void(const VkCommandBuffer&)> func, std::function<void()> callback = {});
	// Schedules graphics queue commands to run at the latest during the next call to present() after any scheduled copy queue commands
	void scheduleOnGraphicsQueue(std::function<void(const VkCommandBuffer&)> func, std::function<void()> callback = {});
	// Submits all scheduled commands to the GPU and then stalls the CPU until work is finished
	void flushScheduledCommands();

	void submitCommandBuffers(std::vector<VkCommandBuffer> cmds);

private:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> copyFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value() && copyFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createViewportAndScissorRect();
	void createFramebuffers();

	void cleanupSwapChain();
	void recreateSwapChain();

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
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window* window) const;


private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	// Viewport and scissor rect
	VkViewport m_viewport;
	VkRect2D m_scissorRect;

	// The following variables should maybe be moved
	VkRenderPass m_renderPass; // maybe not move this?
	VkClearValue m_clearColor;
	VkDescriptorPool m_descriptorPool;

	// Queues
	VkQueue m_queueGraphics;
	VkQueue m_queuePresent;
	VkQueue m_queueCopy;
	std::vector<uint32_t> m_queueFamilyIndicesGraphicsAndCopy;

	VkCommandPool m_commandPoolGraphics;
	VkCommandPool m_commandPoolCopy;
	std::vector<VkCommandBuffer> m_commandBuffersCopy;
	std::vector<VkCommandBuffer> m_commandBuffersGraphics;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;
	size_t m_currentFrame;
	
	// Variables stored by beginFrame() and used by present()
	uint32_t m_presentImageIndex;
	VkResult m_acqureNextImageResult;

	bool m_framebufferResized;
	bool m_isWindowMinimized;
	bool m_isFirstFrame;

	const std::vector<const char*> m_validationLayers;
	const std::vector<const char*> m_deviceExtensions;

	std::vector<std::pair< std::function<void(const VkCommandBuffer&)>, std::function<void()> >> m_scheduledCopyCommandsAndCallbacks;
	std::vector<std::pair< std::function<void(const VkCommandBuffer&)>, std::function<void()> >> m_scheduledGraphicsCommandsAndCallbacks;
	std::vector<VkFence> m_fencesInFlightCopy;
	std::vector<VkFence> m_fencesJustInCaseCopyInFlight;
	std::vector<VkFence> m_fenceScheduledGraphicsCmds;
	std::vector<std::vector<std::function<void()>>> m_executionCallbacksCopy;
	std::vector<std::vector<std::function<void()>>> m_executionCallbacksGraphics;

#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif

	// VulkanMemoryAllocator lib
	VmaAllocator m_vmaAllocator;

};