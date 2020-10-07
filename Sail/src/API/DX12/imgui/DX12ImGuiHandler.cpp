#include "pch.h"
#include "DX12ImGuiHandler.h"
#include "Sail/Application.h"
#include "../DX12API.h"
#include "API/Windows/Win32Window.h"
#include "../resources/DX12Texture.h"

#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx12.h"

// Build imgui examples
#include "examples/imgui_impl_dx12.cpp"
#include "examples/imgui_impl_win32.cpp"

ImGuiHandler* ImGuiHandler::Create() {
	return SAIL_NEW DX12ImGuiHandler();
}

DX12ImGuiHandler::DX12ImGuiHandler() {
	ImGui_ImplWin32_EnableDpiAwareness();
}

DX12ImGuiHandler::~DX12ImGuiHandler() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DX12ImGuiHandler::init() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<DX12API>();
	auto* window = Application::getInstance()->getWindow<Win32Window>();

	// Set up a GPU visible srv descriptor heap
	m_descHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100, true);

	m_context->initCommand(m_command, D3D12_COMMAND_LIST_TYPE_DIRECT, L"imgui command list");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
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
	ImGui_ImplDX12_Init(m_context->getDevice(), DX12API::NUM_SWAP_BUFFERS,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_descHeap->get(),
		m_descHeap->getCPUDescriptorHandleForIndex(0),
		m_descHeap->getGPUDescriptorHandleForIndex(0));
	m_descHeap->setIndex(1);
}

void DX12ImGuiHandler::begin() {
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX12ImGuiHandler::end() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto& allocator = m_command.allocators[m_context->getFrameIndex()];
	auto& cmdList = m_command.list;

	// Reset allocators and lists for this frame
	allocator->Reset();
	cmdList->Reset(allocator.Get(), nullptr);

	// Transition all used textures to pixel shader resource
	for (auto* texture : m_texturesUsedThisFrame) {
		//texture->initBuffers(cmdList.Get()); // Init shoud have happened in the renderer already (?)
		if (texture->isReadyToUse())
			texture->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	m_texturesUsedThisFrame.clear();

	// Transition back buffer to render target
	m_context->prepareToRender(cmdList.Get());

	cmdList->OMSetRenderTargets(1, &m_context->getCurrentRenderTargetCDH(), true, &m_context->getDsvCDH());

	// Set the descriptor heap
	m_descHeap->bind(cmdList.Get());
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

	// Transition back buffer to present
	m_context->prepareToPresent(cmdList.Get());
	// Execute command list
	cmdList->Close();
	m_context->getDirectQueue()->executeCommandLists({ cmdList.Get() });

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Reset descriptor heap index as soon as the SRVs can be overwritten
	// Set it to 1 since the first is always used for imgui fonts
	if (m_context->getSwapIndex() == 1) {
		m_descHeap->setIndex(1);
	}
}

ImTextureID DX12ImGuiHandler::getTextureID(Texture* texture) {
	auto* dxTexture = static_cast<DX12Texture*>(texture);

	auto heapIndex = m_descHeap->getAndStepIndex();
	m_context->getDevice()->CopyDescriptorsSimple(1, m_descHeap->getCPUDescriptorHandleForIndex(heapIndex), dxTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Store texture since its resource state needs to change to PIXEL_SHADER_RESOURCE before rendering
	m_texturesUsedThisFrame.emplace_back(dxTexture);

	return (ImTextureID)m_descHeap->getGPUDescriptorHandleForIndex(heapIndex).ptr;
}