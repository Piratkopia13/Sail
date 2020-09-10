#include "pch.h"
#include "SVkAPI.h"
#include "vulkan/vulkan_win32.h"
#include "../Windows/Win32Window.h"

const int SVkAPI::MAX_FRAMES_IN_FLIGHT = 2;

GraphicsAPI* GraphicsAPI::Create() {
	return SAIL_NEW SVkAPI();
}

SVkAPI::SVkAPI() 
	: m_validationLayers({	"VK_LAYER_KHRONOS_validation"	})
	, m_deviceExtensions({	VK_KHR_SWAPCHAIN_EXTENSION_NAME	})
	, m_physicalDevice(VK_NULL_HANDLE)
	, m_currentFrame(0)
	, m_viewport()
	, m_scissorRect()
	, m_presentImageIndex(-1)
	, m_clearColor({ 0.f, 0.0f, 0.f, 1.f }) // Default clear color
{
	Logger::Log("Initializing Vulkan...");
}

SVkAPI::~SVkAPI() {
	vkDeviceWaitIdle(m_device);

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		vkDestroyFence(m_device, m_fencesInFlightCopy[i], nullptr);
	}
	vkDestroyCommandPool(m_device, m_commandPoolGraphics, nullptr);
	vkDestroyCommandPool(m_device, m_commandPoolCopy, nullptr);
	
	for (auto& framebuffer : m_swapChainFramebuffers) {
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	for (auto& imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	vkDestroyDevice(m_device, nullptr);
	if (m_enableValidationLayers) DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

bool SVkAPI::init(Window* window) {

	auto winWindow = static_cast<Win32Window*>(window);
	
	// Make sure validation layers are available if run in debug
	if (m_enableValidationLayers && !checkValidationLayerSupport()) {
		Logger::Error("Vulkan validation layers requested, but not available!");
		return false;
	}

	// Set up instance info and create it
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Sail";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			Logger::Error("Failed to create Vulkan instance!");
			return false;
		}

		setupDebugMessenger();
	}

	// Set up surface
	{
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = *winWindow->getHwnd();
		createInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
			Logger::Error("Failed to create window surface!");
			return false;
		}
	}

	// Enumerate available extensions
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}
	}

	// Select the graphics card to use
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			Logger::Error("Failed to find GPUs with Vulkan support!");
			return false;
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		auto isDeviceSuitable = [&](const VkPhysicalDevice& device) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			QueueFamilyIndices indices = findQueueFamilies(device);
			bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported) {
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			// TODO: support other gpu types
			return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				indices.isComplete() && 
				extensionsSupported &&
				swapChainAdequate &&
				deviceFeatures.shaderClipDistance && deviceFeatures.samplerAnisotropy;
		};

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE) {
			Logger::Error("Failed to find a suitable GPU!");
			return false;
		}
	}

	// Create the logical device
	{
		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.copyFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{ };
		deviceFeatures.shaderClipDistance = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		// enabledLayerCOunt and ppEnabledLayerNames are obsolete in newer Vulkan implementations
		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
			Logger::Error("Failed to create logical device!");
			return false;
		}

		// Store the queue handle
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_queueGraphics);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_queuePresent);
		vkGetDeviceQueue(m_device, indices.copyFamily.value(), 0, &m_queueCopy);
	}

	// Create the swap chain
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, winWindow);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE; // Set this when recreating the swap chain during for example window resizing

		if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
			Logger::Error("Failed to create swap chain!");
			return false;
		}

		// Store handles to the swapchain images
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
		m_swapChainImageFormat = createInfo.imageFormat;
		m_swapChainExtent = createInfo.imageExtent;
	}

	// Create image views for the swap chain back buffers
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());
		for (size_t i = 0; i < m_swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
				Logger::Error("failed to create image views!");
				return false;
			}
		}
	}



	// Anything below this should be separated from this file asap

	// Create render pass - maybe keep this here?
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Subpasses
		// TODO: use multiple of these for post processing
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		// Add dependency to wait for image reading to complete before writing
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			Logger::Error("failed to create render pass!");
			return false;
		}
	}

	// Create viewport and scissor rect
	{		
		m_viewport.x = 0.0f;
		m_viewport.y = (float)m_swapChainExtent.height; // Flipping viewport Y to work with hlsl shaders
		m_viewport.width = (float)m_swapChainExtent.width;
		m_viewport.height = -(float)m_swapChainExtent.height; // Flipping viewport Y to work with hlsl shaders
		m_viewport.minDepth = 0.0f;
		m_viewport.maxDepth = 1.0f;

		m_scissorRect.offset = { 0, 0 };
		m_scissorRect.extent = m_swapChainExtent;
	}

	// Create framebuffers
	// This may be able to stay in this class
	{
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
		for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				m_swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
				Logger::Error("Failed to create framebuffer!");
				return false;
			}
		}
	}

	// Create command pool
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		// Graphics command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // TODO: check if transient bit is useful here

		if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPoolGraphics) != VK_SUCCESS) {
			Logger::Error("Failed to create graphics command pool!");
			return false;
		}

		// Copy command pool
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.copyFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPoolCopy) != VK_SUCCESS) {
			Logger::Error("Failed to create copy command pool!");
			return false;
		}
	}

	// Create command buffers
	{
		m_commandBuffersCopy.resize(m_swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPoolCopy;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffersCopy.size();

		if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffersCopy.data()) != VK_SUCCESS) {
			Logger::Error("Failed to allocate command buffers!");
			return false;
		}
	}
	// Record drawing into command buffers
	{
		//for (size_t i = 0; i < m_commandBuffers.size(); i++) {
		//	VkCommandBufferBeginInfo beginInfo{};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	beginInfo.flags = 0; // Optional
		//	beginInfo.pInheritanceInfo = nullptr; // Optional

		//	if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
		//		Logger::Error("Failed to begin recording command buffer!");
		//		return false;
		//	}

		//	// Start render pass
		//	VkRenderPassBeginInfo renderPassInfo{};
		//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//	renderPassInfo.renderPass = m_renderPass;
		//	renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
		//	renderPassInfo.renderArea.offset = { 0, 0 };
		//	renderPassInfo.renderArea.extent = m_swapChainExtent;
		//	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		//	renderPassInfo.clearValueCount = 1;
		//	renderPassInfo.pClearValues = &clearColor;

		//	vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//	vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		//	vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

		//	vkCmdEndRenderPass(m_commandBuffers[i]);

		//	if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
		//		Logger::Error("failed to record command buffer!");
		//		return false;
		//	}
		//}
	}

	// Create sync objects
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_fencesInFlightCopy.resize(MAX_FRAMES_IN_FLIGHT);
		m_executionCallbacks.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);
		m_fencesJustInCaseCopyInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS ||
				vkCreateFence(m_device, &fenceInfo, nullptr, &m_fencesInFlightCopy[i]) != VK_SUCCESS) {

				Logger::Error("Failed to create synchronization objects for a frame!");
				return false;
			}
		}
	}

	// Create descriptor pool
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(m_swapChainImages.size()) * 10; // TODO: make this dynamic? or just allocate a bunch

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());

		if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
			Logger::Error("Failed to create descriptor pool!");
		}
	}

	return true;
}

void SVkAPI::clear(const glm::vec4& color) {
	m_clearColor = { color.r, color.g, color.b, color.a };
}

void SVkAPI::setDepthMask(DepthMask setting) { /* Defined the the PSO */ }
void SVkAPI::setFaceCulling(Culling setting) { /* Defined the the PSO */ }
void SVkAPI::setBlending(Blending setting) { /* Defined the the PSO */ }

uint32_t SVkAPI::beginPresent() {
	// Make sure the CPU waits to submit if all frames are in flight
	VkFence waitFences[] = { m_inFlightFences[m_currentFrame], m_fencesInFlightCopy[m_currentFrame] };
	vkWaitForFences(m_device, ARRAYSIZE(waitFences), waitFences, VK_TRUE, UINT64_MAX);

	// At this point we know that execution has finished for m_currentFrame
	// Call any waiting callbacks to let them know
	if (!m_executionCallbacks[m_currentFrame].empty()) {
		for (auto& callback : m_executionCallbacks[m_currentFrame]) {
			callback();
		}
		m_executionCallbacks[m_currentFrame].clear();
	}
	
	vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_presentImageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (m_imagesInFlight[m_presentImageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(m_device, 1, &m_imagesInFlight[m_presentImageIndex], VK_TRUE, UINT64_MAX);
	}
	if (m_fencesJustInCaseCopyInFlight[m_presentImageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(m_device, 1, &m_fencesJustInCaseCopyInFlight[m_presentImageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	m_imagesInFlight[m_presentImageIndex] = m_inFlightFences[m_currentFrame];
	m_fencesJustInCaseCopyInFlight[m_presentImageIndex] = m_fencesInFlightCopy[m_currentFrame];

	return m_presentImageIndex;
}

void SVkAPI::present(bool vsync) {
	// beginPresent fetches the image index to use, the assert makes sure this has been done
	// The image index is used to bind certain buffers which is why it's fetch has to be separated
	assert(m_presentImageIndex != -1 && "beginPresent() has to be called before present() when using Vulkan");

	// Execute any waiting copy commands
	{
		/*if (!m_scheduledCopyCommands.empty())*/ {
			auto& cmdBufferCopy = m_commandBuffersCopy[m_presentImageIndex];
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(cmdBufferCopy, &beginInfo);

			for (auto& pair : m_scheduledCopyCommandsAndCallbacks) {
				// Call the lambda to add commands to the commandBuffer
				pair.first(cmdBufferCopy);
				// Add callback to list
				m_executionCallbacks[m_currentFrame].emplace_back(pair.second);
			}
			m_scheduledCopyCommandsAndCallbacks.clear();

			vkEndCommandBuffer(cmdBufferCopy);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBufferCopy;

			// The fence used is the same fence given to the copy command submitters,
			// this allows them to fetch when their command has been executed
			vkResetFences(m_device, 1, &m_fencesInFlightCopy[m_currentFrame]);
			vkQueueSubmit(m_queueCopy, 1, &submitInfo, m_fencesInFlightCopy[m_currentFrame]);
		}
	}

	// Present
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_presentImageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(m_queuePresent, &presentInfo);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	m_presentImageIndex = -1; // Invalidate this image index
}

unsigned int SVkAPI::getMemoryUsage() const {
	throw std::logic_error("The method or operation is not implemented.");
}

unsigned int SVkAPI::getMemoryBudget() const {
	throw std::logic_error("The method or operation is not implemented.");
}

bool SVkAPI::onResize(WindowResizeEvent& event) {
	// Recreate some stuff that is now invalidated

	vkDeviceWaitIdle(m_device);

	/*createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandBuffers();*/

	throw std::logic_error("The method or operation is not implemented.");
}

const VkDevice& SVkAPI::getDevice() const {
	return m_device;
}

const VkPhysicalDevice& SVkAPI::getPhysicalDevice() const {
	return m_physicalDevice;
}

const VkViewport& SVkAPI::getViewport() const {
	return m_viewport;
}

const VkRect2D& SVkAPI::getScissorRect() const {
	return m_scissorRect;
}

const VkRenderPass& SVkAPI::getRenderPass() const {
	return m_renderPass;
}

uint32_t SVkAPI::getSwapImageIndex() const {
	assert(m_presentImageIndex != -1 && "getSwapImageIndex() has to be called between beginPresent() and present()");
	return m_presentImageIndex;
}

size_t SVkAPI::getNumSwapChainImages() const {
	return m_swapChainImages.size();
}

VkRenderPassBeginInfo SVkAPI::getRenderPassInfo() const {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFramebuffers[getSwapImageIndex()];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &m_clearColor;

	return renderPassInfo;
}

const VkDescriptorPool& SVkAPI::getDescriptorPool() const {
	return m_descriptorPool;
}

void SVkAPI::initCommand(Command& command) const {
	command.buffers.resize(m_swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPoolGraphics;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)command.buffers.size();

	if (vkAllocateCommandBuffers(m_device, &allocInfo, command.buffers.data()) != VK_SUCCESS) {
		Logger::Error("Failed to allocate command buffers!");
	}
}

void SVkAPI::scheduleMemoryCopy(std::function<void(const VkCommandBuffer&)> func, std::function<void()> callback) {
	m_scheduledCopyCommandsAndCallbacks.emplace_back(std::make_pair(func, callback));
	//return m_fencesInFlightCopy[m_currentFrame];
}

void SVkAPI::submitCommandBuffers(std::vector<VkCommandBuffer> cmds) {
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(cmds.size());
	submitInfo.pCommandBuffers = cmds.data();

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	// NOTE: only graphics queue is currently used
	if (vkQueueSubmit(m_queueGraphics, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		Logger::Error("Failed to submit draw command buffer!");
	}
}

bool SVkAPI::checkValidationLayerSupport() const {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> SVkAPI::getRequiredExtensions() const {
	// TODO: support glfw / cross platform windows
	const char* extensionNames[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };

	std::vector<const char*> extensions(extensionNames, extensionNames + ARRAYSIZE(extensionNames));

	// Add validation layer extension if requested
	if (m_enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void SVkAPI::setupDebugMessenger() {
	if (!m_enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr; // Optional

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		Logger::Error("Failed to set up debug messenger");
	}
}

SVkAPI::QueueFamilyIndices SVkAPI::findQueueFamilies(const VkPhysicalDevice& device) const {
	QueueFamilyIndices indices;
	// Assign index to queue families that could be found
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		} else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.copyFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}

bool SVkAPI::checkDeviceExtensionSupport(const VkPhysicalDevice& device) const {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SVkAPI::SwapChainSupportDetails SVkAPI::querySwapChainSupport(const VkPhysicalDevice& device) const {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
	
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR SVkAPI::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SVkAPI::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SVkAPI::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Win32Window* window) const {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = { window->getWindowWidth(), window->getWindowHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkShaderModule SVkAPI::createShaderModule(const std::vector<std::byte>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		Logger::Error("Failed to create shader module!");
	}

	return shaderModule;
}

VkResult SVkAPI::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void SVkAPI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL SVkAPI::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	OutputDebugStringA(pCallbackData->pMessage);
	OutputDebugStringA("\n");

	std::string errMsg = "Validation layer:\n\t";
	errMsg += pCallbackData->pMessage;
	Logger::Warning(errMsg+"\n");
	//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}