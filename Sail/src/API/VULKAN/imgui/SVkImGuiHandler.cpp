#include "pch.h"
#include "SVkImGuiHandler.h"
#include "Sail/Application.h"
#include "API/Windows/Win32Window.h"
#include "../SVkUtils.h"

#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_vulkan.h"

// Build imgui examples
#include "examples/imgui_impl_vulkan.cpp"
#include "examples/imgui_impl_win32.cpp"

ImGuiHandler* ImGuiHandler::Create() {
	return SAIL_NEW SVkImGuiHandler();
}

SVkImGuiHandler::SVkImGuiHandler() {
	ImGui_ImplWin32_EnableDpiAwareness();
}

SVkImGuiHandler::~SVkImGuiHandler() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	vkDestroyRenderPass(m_context->m_device, m_renderPass, nullptr);
	vkDestroyDescriptorPool(m_context->m_device, m_descriptorPool, nullptr);
}

void SVkImGuiHandler::init() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<SVkAPI>();
	auto* window = Application::getInstance()->getWindow<Win32Window>();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingTransparentPayload = true;

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Dear ImGui style
	applySailStyle();

	// Merge icons into normal font
	//io.Fonts->AddFontDefault();
	const std::string defaultPath = "res/fonts/";
	float fontSize = 32.f;
	io.Fonts->AddFontFromFileTTF(std::string(defaultPath + "OpenSans-SemiBold.ttf").c_str(), fontSize);
	io.FontGlobalScale = 16.f / fontSize;
	//io.FontAllowUserScaling = true;

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = fontSize; // Use if you want to make the icon monospaced
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF("res/fonts/fontawesome-webfont.ttf", fontSize, &config, icon_ranges);

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init((void*)*window->getHwnd());


	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK_RESULT(vkCreateDescriptorPool(m_context->m_device, &pool_info, nullptr, &m_descriptorPool));
	}
	// Create render pass
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_context->m_swapchainImageFormat;
		colorAttachment.samples = m_context->m_msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = m_context->m_depthImageFormat;
		depthAttachment.samples = m_context->m_msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = m_context->m_swapchainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		if (m_context->m_msaaSamples > 1)
			subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
		if (m_context->m_msaaSamples > 1)
			attachments.emplace_back(colorAttachmentResolve);
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(m_context->m_device, &renderPassInfo, nullptr, &m_renderPass));
	}


	auto checkVkResult = [](VkResult err) {
		VK_CHECK_RESULT(err);
	};
	
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = m_context->m_instance;
	initInfo.PhysicalDevice = m_context->m_physicalDevice;
	initInfo.Device = m_context->m_device;
	initInfo.QueueFamily = m_context->m_queueFamilyIndicesGraphicsAndCopy[0];
	initInfo.Queue = m_context->m_queueGraphics;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = m_descriptorPool;
	initInfo.Allocator = VK_NULL_HANDLE;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = m_context->getNumSwapBuffers();
	initInfo.CheckVkResultFn = checkVkResult;
	initInfo.MSAASamples = m_context->m_msaaSamples;
	ImGui_ImplVulkan_Init(&initInfo, m_renderPass);

	// Load fonts
	m_context->scheduleOnGraphicsQueue([&](const VkCommandBuffer& cmd) {
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	});
}

void SVkImGuiHandler::begin() {
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void SVkImGuiHandler::end() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	ImGui::Render();
	ImDrawData* mainDrawData = ImGui::GetDrawData();
	const bool mainIsMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);
	
	if (!mainIsMinimized) {

		m_context->scheduleOnGraphicsQueue([&, mainDrawData](const VkCommandBuffer& cmd) {

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_context->m_swapchainFramebuffers[m_context->getSwapIndex()];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_context->m_swapchainExtent;

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(mainDrawData, cmd);

			vkCmdEndRenderPass(cmd);
		});

	}

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

}

ImTextureID SVkImGuiHandler::getTextureID(Texture* texture) {
	assert(false);
	return 0;
}