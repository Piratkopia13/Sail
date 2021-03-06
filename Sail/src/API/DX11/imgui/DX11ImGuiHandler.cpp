#include "pch.h"
#include "DX11ImGuiHandler.h"
#include "Sail/Application.h"
#include "../DX11API.h"
#include "API/Windows/Win32Window.h"

#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"

// Build imgui examples
#include "examples/imgui_impl_dx11.cpp"
#include "examples/imgui_impl_win32.cpp"



ImGuiHandler* ImGuiHandler::Create() {
	return SAIL_NEW DX11ImGuiHandler();
}

DX11ImGuiHandler::DX11ImGuiHandler() {
	ImGui_ImplWin32_EnableDpiAwareness();
}

DX11ImGuiHandler::~DX11ImGuiHandler() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DX11ImGuiHandler::init() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
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
	const std::string defaultPath = "res/fonts/";
	float fontSize = 32.f;
	io.Fonts->AddFontFromFileTTF(std::string(defaultPath + "OpenSans-SemiBold.ttf").c_str(), fontSize);
	io.FontGlobalScale = 16.f / fontSize;

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = fontSize; // Use if you want to make the icon monospaced
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF("res/fonts/fontawesome-webfont.ttf", fontSize, &config, icon_ranges);

	auto* api = Application::getInstance()->getAPI<DX11API>();
	auto* window = Application::getInstance()->getWindow<Win32Window>();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init((void*) * (window->getHwnd()));
	ImGui_ImplDX11_Init(api->getDevice(), api->getDeviceContext());
}

void DX11ImGuiHandler::begin() {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX11ImGuiHandler::end() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	Application::getInstance()->getAPI<DX11API>()->renderToBackBuffer(); // This is only here because imgui changes render target
}

ImTextureID DX11ImGuiHandler::getTextureID(Texture* texture) {
	assert(false && "ImGui texture rendering is currently not supported for DX11");
	return 0;
}
