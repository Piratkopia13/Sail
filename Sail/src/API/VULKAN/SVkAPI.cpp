﻿#include "pch.h"
#include "SVkAPI.h"
#include "vulkan/vulkan_win32.h"
#include "../Windows/Win32Window.h"
#include "SVkUtils.h"

const int SVkAPI::MAX_FRAMES_IN_FLIGHT = 2;

GraphicsAPI* GraphicsAPI::Create() {
	return SAIL_NEW SVkAPI();
}

SVkAPI::SVkAPI() 
	: m_validationLayers({	"VK_LAYER_KHRONOS_validation"	})
	, m_deviceExtensions({	VK_KHR_SWAPCHAIN_EXTENSION_NAME	})
	, m_physicalDevice(VK_NULL_HANDLE)
	, m_colorImageView(VK_NULL_HANDLE)
	, m_currentFrame(0)
	, m_viewport()
	, m_scissorRect()
	, m_presentImageIndex(-1)
	, m_framebufferResized(false)
	, m_isWindowMinimized(false)
	, m_isFirstFrame(true)
	, m_hasSubmittedThisFrame(false)
	, m_msaaSamples(VK_SAMPLE_COUNT_1_BIT)
{
	m_clearValues[0] = { 0.f, 0.0f, 0.f, 1.f }; // Default clear color
	m_clearValues[1] = { 1.f, 0.f }; // Default clear depth

	Logger::Log("Initializing Vulkan...");
}

SVkAPI::~SVkAPI() {
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		vkDestroyFence(m_device, m_fencesInFlightCopy[i], nullptr);
		vkDestroyFence(m_device, m_fenceScheduledGraphicsCmds[i], nullptr);
	}
	cleanupSwapchain();
	vmaDestroyAllocator(m_vmaAllocator);

	vkDestroyCommandPool(m_device, m_commandPoolGraphics, nullptr);
	vkDestroyCommandPool(m_device, m_commandPoolCopy, nullptr);

	vkDestroyDevice(m_device, nullptr);
	if (m_enableValidationLayers) DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

bool SVkAPI::init(Window* window) {
	
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

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));

		setupDebugMessenger();
	}

	// Set up surface
	{
		auto winWindow = static_cast<Win32Window*>(window);
		
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = *winWindow->getHwnd();
		createInfo.hinstance = GetModuleHandle(nullptr);

		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface));
	}

	// Enumerate available extensions
	{
		uint32_t extensionCount = 0;
		VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		std::vector<VkExtensionProperties> extensions(extensionCount);
		VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));

		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}
	}

	// Select the graphics card to use
	{
		uint32_t deviceCount = 0;
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
		if (deviceCount == 0) {
			Logger::Error("Failed to find GPUs with Vulkan support!");
			return false;
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

		auto isDeviceSuitable = [&](const VkPhysicalDevice& device) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			Logger::Log("maxPushConstantsSize: " + std::to_string(deviceProperties.limits.maxPushConstantsSize));
			Logger::Log("minUniformBufferOffsetAlignment: " + std::to_string(deviceProperties.limits.minUniformBufferOffsetAlignment));

			QueueFamilyIndices indices = findQueueFamilies(device);
			bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported) {
				SwapchainSupportDetails swapChainSupport = querySwapchainSupport(device);
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
				m_msaaSamples = getMaxUsableSampleCount();

				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				Logger::Log("Using device " + std::string(deviceProperties.deviceName));

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
		m_queueFamilyIndicesGraphicsAndCopy.emplace_back(indices.graphicsFamily.value());
		m_queueFamilyIndicesGraphicsAndCopy.emplace_back(indices.copyFamily.value());

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

		//VkPhysicalDeviceRobustness2FeaturesEXT deviceRobustnessFeatures{};
		//deviceRobustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		//deviceRobustnessFeatures.nullDescriptor = VK_TRUE; // Allow null descriptors! Used for images before they are ready to be read in shaders

		VkPhysicalDeviceDescriptorIndexingFeatures deviceDescIndexFeatures{};
		deviceDescIndexFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		deviceDescIndexFeatures.runtimeDescriptorArray = VK_TRUE;

		VkPhysicalDeviceFeatures2 deviceFeatures {};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.features.shaderClipDistance = VK_TRUE;
		deviceFeatures.features.sampleRateShading = VK_TRUE;
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;
		//deviceFeatures.features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
		deviceFeatures.pNext = &deviceDescIndexFeatures;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = nullptr;
		createInfo.pNext = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		// enabledLayerCOunt and ppEnabledLayerNames are obsolete in newer Vulkan implementations
		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		VK_CHECK_RESULT(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

		// Store the queue handle
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_queueGraphics);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_queuePresent);
		vkGetDeviceQueue(m_device, indices.copyFamily.value(), 0, &m_queueCopy);
	}

	// Set up Vulkan Memory Allocator
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_physicalDevice;
		allocatorInfo.device = m_device;
		allocatorInfo.instance = m_instance;

		if (vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator) != VK_SUCCESS) {
			Logger::Error("Failed to create vma allocator!");
			return false;
		}
	}

	createSwapchain();

	createImageViews();

	// keep this here? or remove it?
	createRenderPass();

	createViewportAndScissorRect();

	createColorResources();

	createDepthResources();

	createFramebuffers();

	// Create command pool
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		// Graphics command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // TODO: check if transient bit is useful here

		VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPoolGraphics));

		// Copy command pool
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.copyFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPoolCopy));
	}

	// Create command buffers used for the copy queue
	{
		m_commandBuffersCopy.resize(m_swapchainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPoolCopy;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffersCopy.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffersCopy.data()));
	}
	// Create command buffers used for the graphics queue
	{
		m_commandBuffersGraphics.resize(m_swapchainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPoolGraphics;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffersGraphics.size();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffersGraphics.data()));

		m_commandBuffersLastGraphics.resize(m_swapchainFramebuffers.size());
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffersLastGraphics.data()));
	}

	// Create sync objects
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_fencesInFlightCopy.resize(MAX_FRAMES_IN_FLIGHT);
		m_fenceScheduledGraphicsCmds.resize(MAX_FRAMES_IN_FLIGHT);
		m_executionCallbacksCopy.resize(MAX_FRAMES_IN_FLIGHT);
		m_executionCallbacksGraphics.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);
		m_fencesJustInCaseCopyInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
			VK_CHECK_RESULT(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]));
			VK_CHECK_RESULT(vkCreateFence(m_device, &fenceInfo, nullptr, &m_fencesInFlightCopy[i]));
			VK_CHECK_RESULT(vkCreateFence(m_device, &fenceInfo, nullptr, &m_fenceScheduledGraphicsCmds[i]));
		}
	}

	// Create descriptor pool
	{
		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size()) * 10000; // TODO: make these dynamic? or just allocate a bunch
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size()) * 10000;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size()) * 10000;
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size()) * 10000;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_swapchainImages.size()) * 10000;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allows freeing descriptor sets, this is required for shader hot reloading to work

		VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool));
	}

	return true;
}

void SVkAPI::clear(const glm::vec4& color) {
	m_clearValues[0] = { color.r, color.g, color.b, color.a };
}

void SVkAPI::setDepthMask(DepthMask setting) { /* Defined the the PSO */ }
void SVkAPI::setFaceCulling(Culling setting) { /* Defined the the PSO */ }
void SVkAPI::setBlending(Blending setting) { /* Defined the the PSO */ }

void SVkAPI::waitForGPU() {
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));
}

uint32_t SVkAPI::getNumSwapBuffers() const {
	return m_swapchainImages.size();
}

uint32_t SVkAPI::getSwapIndex() const {
	assert(m_presentImageIndex != -1 && "getSwapImageIndex() has to be called between beginPresent() and present()");
	return m_presentImageIndex;
}

void SVkAPI::beginPresent() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	EventSystem::getInstance()->dispatchEvent(NewFrameEvent());

	if (m_isFirstFrame) {
		// Flush and scheduled commands on the first frame.
		// This makes sure that the missing texture is available on the first draw call.
		flushScheduledCommands();
		m_isFirstFrame = false;
	}

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Wait for fences");

		// Make sure the CPU waits to submit if all frames are in flight
		VkFence waitFences[] = { m_inFlightFences[m_currentFrame], m_fencesInFlightCopy[m_currentFrame] };
		VK_CHECK_RESULT(vkWaitForFences(m_device, ARRAYSIZE(waitFences), waitFences, VK_TRUE, UINT64_MAX));
	}

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Run callbacks");

		// At this point we know that execution has finished for m_currentFrame
			// Call any waiting callbacks to let them know
		if (!m_executionCallbacksCopy[m_currentFrame].empty()) {
			for (auto& callback : m_executionCallbacksCopy[m_currentFrame]) {
				if (callback) callback();
			}
			m_executionCallbacksCopy[m_currentFrame].clear();
		}
		// TODO: check if this m_fenceScheduledGraphicsCmds fence is really required
		if (vkGetFenceStatus(m_device, m_fenceScheduledGraphicsCmds[m_currentFrame]) == VK_SUCCESS) {
			for (auto& callback : m_executionCallbacksGraphics[m_currentFrame]) {
				if (callback) callback();
			}
			m_executionCallbacksGraphics[m_currentFrame].clear();
		}
	}
	
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Acquire next image");
		m_acqureNextImageResult = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_presentImageIndex);
	}

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Wait for fences");

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (m_imagesInFlight[m_presentImageIndex] != VK_NULL_HANDLE) {
			VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_imagesInFlight[m_presentImageIndex], VK_TRUE, UINT64_MAX));
		}
		if (m_fencesJustInCaseCopyInFlight[m_presentImageIndex] != VK_NULL_HANDLE) {
			VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_fencesJustInCaseCopyInFlight[m_presentImageIndex], VK_TRUE, UINT64_MAX));
		}
	}
	// Mark the image as now being in use by this frame
	m_imagesInFlight[m_presentImageIndex] = m_inFlightFences[m_currentFrame];
	m_fencesJustInCaseCopyInFlight[m_presentImageIndex] = m_fencesInFlightCopy[m_currentFrame];
}

void SVkAPI::present(bool vsync) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// beginPresent fetches the image index to use, the assert makes sure this has been done
	// The image index is used to bind certain buffers which is why it's fetch has to be separated
	assert(m_presentImageIndex != -1 && "beginPresent() has to be called before present() when using Vulkan");

	if (m_acqureNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	} else if (m_acqureNextImageResult != VK_SUCCESS && m_acqureNextImageResult != VK_SUBOPTIMAL_KHR) {
		Logger::Error("Failed to acquire swap chain image!");
	}

	// Execute any waiting copy commands
	// NOTE: these do not wait for the last "same frame index" to finish rendering if no submitCommandBuffers() call was made this frame
	{
		if (!m_scheduledCopyCommandsAndCallbacks.empty()) {
			auto& cmdBufferCopy = m_commandBuffersCopy[m_presentImageIndex];
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBufferCopy, &beginInfo));

			for (auto& pair : m_scheduledCopyCommandsAndCallbacks) {
				// Call the lambda to add commands to the commandBuffer
				pair.first(cmdBufferCopy);
				// Add callback to list
				m_executionCallbacksCopy[m_currentFrame].emplace_back(pair.second);
			}
			m_scheduledCopyCommandsAndCallbacks.clear();

			VK_CHECK_RESULT(vkEndCommandBuffer(cmdBufferCopy));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBufferCopy;

			VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_fencesInFlightCopy[m_currentFrame]));
			VK_CHECK_RESULT(vkQueueSubmit(m_queueCopy, 1, &submitInfo, m_fencesInFlightCopy[m_currentFrame]));
		}
	}
	// Execute any waiting graphics commands
	// NOTE: these do not wait for the last "same frame index" to finish rendering if no submitCommandBuffers() call was made this frame
	{
		if (!m_scheduledGraphicsCommandsAndCallbacks.empty()) {
			auto& cmdBufferGraphics = m_commandBuffersGraphics[m_presentImageIndex];
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBufferGraphics, &beginInfo));

			for (auto& pair : m_scheduledGraphicsCommandsAndCallbacks) {
				// Call the lambda to add commands to the commandBuffer
				pair.first(cmdBufferGraphics);
				// Add callback to list
				m_executionCallbacksGraphics[m_currentFrame].emplace_back(pair.second);
			}
			m_scheduledGraphicsCommandsAndCallbacks.clear();

			VK_CHECK_RESULT(vkEndCommandBuffer(cmdBufferGraphics));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBufferGraphics;

			VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_fenceScheduledGraphicsCmds[m_currentFrame]));
			VK_CHECK_RESULT(vkQueueSubmit(m_queueGraphics, 1, &submitInfo, m_fenceScheduledGraphicsCmds[m_currentFrame]));
		}
	}

	// Execute one last graphics command buffer
	// This transitions the backbuffer layout to be ready for present
	// When this transition is done, we know that the frame is finished on the GPU - therefore the renderFinishedSemaphore is used
	{
		auto& cmdLastGraphics = m_commandBuffersLastGraphics[m_presentImageIndex];
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdLastGraphics, &beginInfo));

		SVkUtils::TransitionImageLayout(cmdLastGraphics, m_swapchainImages[m_presentImageIndex], m_swapchainImageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		VK_CHECK_RESULT(vkEndCommandBuffer(cmdLastGraphics));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdLastGraphics;

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]));

		VK_CHECK_RESULT(vkQueueSubmit(m_queueGraphics, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

	}


	// Present
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_presentImageIndex;
	presentInfo.pResults = nullptr; // Optional

	VkResult result = vkQueuePresentKHR(m_queuePresent, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		recreateSwapchain();
	} else if (result != VK_SUCCESS) {
		Logger::Error("Failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	m_presentImageIndex = -1; // Invalidate this image index
	m_hasSubmittedThisFrame = false;
}

unsigned int SVkAPI::getMemoryUsage() const {
	VmaBudget budget;
	vmaGetBudget(m_vmaAllocator, &budget);
	return budget.usage;
}

unsigned int SVkAPI::getMemoryBudget() const {
	VmaBudget budget;
	vmaGetBudget(m_vmaAllocator, &budget);
	return budget.budget;
}

bool SVkAPI::onResize(WindowResizeEvent& event) {
	// Vulkan max extent capability is (0,0) when window is minimized
	// however, the swapchain can not be created with 0 in any extent
	// we must therefore wait until the window is unminimized
	auto* window = Application::getInstance()->getWindow();
	if (window->getWindowWidth() == 0 | window->getWindowHeight() == 0 || window->isMinimized()) {
		Application::getInstance()->pauseRendering(true);
		m_isWindowMinimized = true;
		return true;
	}

	if (m_isWindowMinimized && !event.isMinimized()) {
		Application::getInstance()->pauseRendering(false);
		m_isWindowMinimized = false;
	} else {
		m_framebufferResized = true;
	}
	return true;
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

VkRenderPassBeginInfo SVkAPI::getRenderPassInfo() const {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[getSwapIndex()];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	
	renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
	renderPassInfo.pClearValues = m_clearValues.data();

	return renderPassInfo;
}

const VkDescriptorPool& SVkAPI::getDescriptorPool() const {
	return m_descriptorPool;
}

const VmaAllocator& SVkAPI::getVmaAllocator() const {
	return m_vmaAllocator;
}

const uint32_t* SVkAPI::getGraphicsAndCopyQueueFamilyIndices() const {
	return m_queueFamilyIndicesGraphicsAndCopy.data();
}

VkSampleCountFlagBits SVkAPI::getSampleCount() const {
	return m_msaaSamples;
}

VkFormat SVkAPI::getDepthFormat() const {
	return m_depthImageFormat;
}

VkFormat SVkAPI::getSwapchainImageFormat() const {
	return m_swapchainImageFormat;
}

const VkFramebuffer& SVkAPI::getCurrentFramebuffer() const {
	return m_swapchainFramebuffers[m_presentImageIndex];
}

const VkExtent2D& SVkAPI::getCurrentExtent() const {
	return m_swapchainExtent;
}

const VkImageView& SVkAPI::getDepthView() const {
	return m_depthImageView;
}

void SVkAPI::initCommand(Command& command) const {
	command.buffers.resize(m_swapchainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPoolGraphics;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)command.buffers.size();

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, command.buffers.data()));
}

void SVkAPI::scheduleOnCopyQueue(std::function<void(const VkCommandBuffer&)> func, std::function<void()> callback) {
	m_scheduledCopyCommandsAndCallbacks.emplace_back(std::make_pair(func, callback));
}

void SVkAPI::scheduleOnGraphicsQueue(std::function<void(const VkCommandBuffer&)> func, std::function<void()> callback) {
	m_scheduledGraphicsCommandsAndCallbacks.emplace_back(std::make_pair(func, callback));
}

void SVkAPI::flushScheduledCommands() {
	std::vector<std::function<void()>> callbacks;

	// Execute any waiting copy commands
	{
		if (!m_scheduledCopyCommandsAndCallbacks.empty()) {
			// Create a temporary command buffer to use
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_commandPoolCopy;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmdBuffer;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, &cmdBuffer));

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

			for (auto& pair : m_scheduledCopyCommandsAndCallbacks) {
				// Call the lambda to add commands to the commandBuffer
				pair.first(cmdBuffer);
				// Add callback to list
				callbacks.emplace_back(pair.second);
			}
			m_scheduledCopyCommandsAndCallbacks.clear();

			VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			VK_CHECK_RESULT(vkQueueSubmit(m_queueCopy, 1, &submitInfo, nullptr));
		}
	}
	// Execute any waiting graphics commands
	{
		if (!m_scheduledGraphicsCommandsAndCallbacks.empty()) {
			// Create a temporary command buffer to use
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_commandPoolGraphics;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer cmdBuffer;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, &cmdBuffer));

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

			for (auto& pair : m_scheduledGraphicsCommandsAndCallbacks) {
				// Call the lambda to add commands to the commandBuffer
				pair.first(cmdBuffer);
				// Add callback to list
				callbacks.emplace_back(pair.second);
			}
			m_scheduledGraphicsCommandsAndCallbacks.clear();

			VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			VK_CHECK_RESULT(vkQueueSubmit(m_queueGraphics, 1, &submitInfo, nullptr));
		}
	}

	// Stall CPU until GPU is finished
	waitForGPU();
	// Call all the callbacks
	for (auto& cb : callbacks) {
		if (cb) cb();
	}
}

void SVkAPI::submitCommandBuffers(std::vector<VkCommandBuffer> cmds) {
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// NOTE: if this method is called multiple times in one frame, the waitSemaphore only needs to be set on the first call
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = (m_hasSubmittedThisFrame) ? 0 : 1; // Only wait on the first submit, the rest will implicitly wait because of order preservation
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(cmds.size());
	submitInfo.pCommandBuffers = cmds.data();

	// NOTE: only graphics queue is currently used
	VK_CHECK_RESULT(vkQueueSubmit(m_queueGraphics, 1, &submitInfo, VK_NULL_HANDLE));
	m_hasSubmittedThisFrame = true;
}

void SVkAPI::createSwapchain() {
	SwapchainSupportDetails swapChainSupport = querySwapchainSupport(m_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, Application::getInstance()->getWindow());

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
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.copyFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 3;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = &queueFamilyIndices[1]; // graphics and copy families
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE; // Set this when recreating the swap chain during for example window resizing

	VK_CHECK_RESULT(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain));

	// Store handles to the swapchain images
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));
	m_swapchainImages.resize(imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data()));
	m_swapchainImageFormat = createInfo.imageFormat;
	m_swapchainExtent = createInfo.imageExtent;

	m_depthImageFormat = chooseDepthFormat();
}

void SVkAPI::createImageViews() {
	m_swapchainImageViews.resize(m_swapchainImages.size());
	for (size_t i = 0; i < m_swapchainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]));
	}
}

void SVkAPI::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = m_msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // There is always a transition to LAYOUT_PRESENT happening right before present

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = m_depthImageFormat;
	depthAttachment.samples = m_msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	//depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = m_swapchainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthAttachmentRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	VkAttachmentReference colorAttachmentResolveRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	// Subpasses
	// TODO: use multiple of these for post processing
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if (m_msaaSamples > 1) 
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

	// Render pass
	std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
	if (m_msaaSamples > 1)
		attachments.emplace_back(colorAttachmentResolve);

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
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

	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}

void SVkAPI::createViewportAndScissorRect() {
	m_viewport.x = 0.0f;
	m_viewport.y = (float)m_swapchainExtent.height; // Flipping viewport Y to work with hlsl shaders
	m_viewport.width = (float)m_swapchainExtent.width;
	m_viewport.height = -(float)m_swapchainExtent.height; // Flipping viewport Y to work with hlsl shaders
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_scissorRect.offset = { 0, 0 };
	m_scissorRect.extent = m_swapchainExtent;
}

void SVkAPI::createColorResources() {
	if (m_msaaSamples <= 1) return;

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = m_swapchainExtent.width;
	imageInfo.extent.height = m_swapchainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = m_swapchainImageFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = m_msaaSamples;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VK_CHECK_RESULT(vmaCreateImage(m_vmaAllocator, &imageInfo, &allocInfo, &m_colorImage.image, &m_colorImage.allocation, nullptr));

	// Create the image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_colorImage.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &m_colorImageView));
}

void SVkAPI::createDepthResources() {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = m_swapchainExtent.width;
	imageInfo.extent.height = m_swapchainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = m_depthImageFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = m_msaaSamples;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VK_CHECK_RESULT(vmaCreateImage(m_vmaAllocator, &imageInfo, &allocInfo, &m_depthImage.image, &m_depthImage.allocation, nullptr));

	// Create the image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_depthImage.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &m_depthImageView));
}

void SVkAPI::createFramebuffers() {
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
	for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
		std::vector<VkImageView> attachments;
		if (m_msaaSamples > 1) {
			attachments.emplace_back(m_colorImageView);
			attachments.emplace_back(m_depthImageView);
			attachments.emplace_back(m_swapchainImageViews[i]);
		} else {
			attachments.emplace_back(m_swapchainImageViews[i]);
			attachments.emplace_back(m_depthImageView);
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]));
	}
}

void SVkAPI::cleanupSwapchain() {
	for (size_t i = 0; i < m_swapchainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, m_swapchainFramebuffers[i], nullptr);
	}

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	m_depthImage.destroy();
	m_colorImage.destroy();
	vkDestroyImageView(m_device, m_depthImageView, nullptr);
	if (m_colorImageView)
		vkDestroyImageView(m_device, m_colorImageView, nullptr);
}

void SVkAPI::recreateSwapchain() {
	// NOTE: It is important to call this after vkQueuePresentKHR to ensure that the semaphores are in a consistent state,
	//       otherwise a signalled semaphore may never be properly waited upon.Now to actually detect resizes we can use the

	Logger::Log("Recreating swap chain..");
	VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));

	cleanupSwapchain();

	createSwapchain();
	createImageViews();
	createRenderPass();
	createViewportAndScissorRect();
	createColorResources();
	createDepthResources();
	createFramebuffers();

	EventSystem::getInstance()->dispatchEvent(SwapchainRecreatedEvent());
}

bool SVkAPI::checkValidationLayerSupport() const {
	uint32_t layerCount;
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	std::vector<VkLayerProperties> availableLayers(layerCount);
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

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
	const char* extensionNames[] = { "VK_KHR_surface", "VK_KHR_win32_surface"/*, "VK_KHR_get_physical_device_properties2", "VK_EXT_memory_budget"*/ };

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
	// GENERAL_BIT is disabled due to my latest driver for some reason spamming the output when calling vkEnumerateDeviceExtensionProperties
	createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr; // Optional

	VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
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
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport));
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
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SVkAPI::SwapchainSupportDetails SVkAPI::querySwapchainSupport(const VkPhysicalDevice& device) const {
	SwapchainSupportDetails details;

	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities));
	
	uint32_t formatCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data()));
	}

	uint32_t presentModeCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data()));
	}

	return details;
}

VkSampleCountFlagBits SVkAPI::getMaxUsableSampleCount() const {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	int c = (int)counts;
	int samplesInt = 1;
	while (c >>= 1) samplesInt <<= 1;
	int requestedSamples = Application::getInstance()->getSettings().getInt(Settings::Graphics_MSAA);
	if (requestedSamples > samplesInt) Logger::Warning("Device only supports " + std::to_string(samplesInt) + "x MSAA, but " + std::to_string(requestedSamples) + "x was requested by the settings.");
	samplesInt = std::min(samplesInt, requestedSamples);

	return (VkSampleCountFlagBits)samplesInt;
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

VkExtent2D SVkAPI::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window* window) const {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = { window->getWindowWidth(), window->getWindowHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkFormat SVkAPI::chooseDepthFormat() {
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats) {
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return format;
		}
	}

	Logger::Error("No depth format supported by the device. say wut?");
	return VK_FORMAT_UNDEFINED;
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