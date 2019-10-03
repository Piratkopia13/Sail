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
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingTabBarOnSingleWindows = true;
	//io.ConfigDockingTransparentPayload = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

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
