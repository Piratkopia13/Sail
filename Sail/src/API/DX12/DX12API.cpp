#include "pch.h"
#include "DX12API.h"
#include "API/Windows/Win32Window.h"
#include "DX12Utils.h"
#include "resources/DescriptorHeap.h"
#include "Sail/Application.h"
#include "Sail/events/Events.h"
#include <iomanip>
#include "Sail/api/shader/Sampler.h"

const UINT DX12API::NUM_SWAP_BUFFERS = 3;
const UINT DX12API::NUM_GPU_BUFFERS = 2;

GraphicsAPI* GraphicsAPI::Create() {
	return SAIL_NEW DX12API();
}

DX12API::DX12API()
	: m_backBufferIndex(0)
	, m_swapIndex(0)
	, m_clearColor{0.8f, 0.2f, 0.2f, 1.0f}
	, m_tearingSupport(true)
	, m_windowedMode(true)
	, m_directQueueFenceValues()
	, m_frameCount(0)
{
	m_renderTargets.resize(NUM_SWAP_BUFFERS);
}

DX12API::~DX12API() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Ensure that the GPU is no longer referencing resources that are about to be destroyed.
	waitForGPU();

#ifdef _DEBUG
	{
		m_dxgiFactory.Reset();
		m_swapChain.Reset();
		m_globalRootSignature.Reset();
		m_renderTargetsHeap.Reset();
		m_depthStencilBuffer.Reset();
		m_directCommandQueue->reset();
		m_dsDescriptorHeap.Reset();
		for (auto& rt : m_renderTargets) {
			rt.Reset();
		}
		m_device.Reset();
		m_cbvSrvUavDescriptorHeap.reset();

		wComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
			OutputDebugString(L"\n========= Live object report =========\n");
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			OutputDebugString(L"======================================\n\n");
		}
	}
#endif
	
}

bool DX12API::init(Window* window) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto winWindow = static_cast<Win32Window*>(window);
	createDevice();
	createCmdInterfacesAndSwapChain(winWindow);
	createEventHandle();
	createRenderTargets();
	createShaderResources();
	createGlobalRootSignature();
	createDepthStencilResources(winWindow);
	createViewportAndScissorRect(winWindow);

	OutputDebugString(L"DX12 Initialized.\n");

	return true;
}


void DX12API::createDevice() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	DWORD dxgiFactoryFlags = 0;
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	wComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
	}
	wComPtr<IDXGIInfoQueue> dxgiInfoQueue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
		dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
	}
	ThrowIfFailed(
		CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf()))
	);
	// PIX programmic capture control
	// Will fail if program is not launched from pix
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_pixGa));
#endif

	// 2. Find comlient adapter and create device

	// dxgi1_6 is only needed for the initialization process using the adapter
	m_factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;
	// Create a factory and iterate through all available adapters 
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		adapter = nullptr;

		if (DXGI_ERROR_NOT_FOUND == m_factory->EnumAdapters1(adapterIndex, &adapter)) {
			break; // No more adapters to enumerate
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr))) {
			break;
		}
		Memory::SafeRelease(&adapter);
	}
	if (adapter) {
		// Create the actual device
		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));

		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);
		std::wstring wdesc(adapterDesc.Description);

		std::string description(wdesc.begin(), wdesc.end());
		float dedicatedVideoMemory = adapterDesc.DedicatedVideoMemory / 1073741824.0f;
		float dedicatedSystemMemory = adapterDesc.DedicatedSystemMemory / 1073741824.0f;
		float sharedSystemMemory = adapterDesc.SharedSystemMemory / 1073741824.0f;

		std::cout << "GPU info:" << std::endl;
		std::cout << "\tDesc: " << description << std::endl;
		std::cout << "\tDedicatedVideoMem: " << dedicatedVideoMemory << std::endl;
		std::cout << "\tDedicatedSystemMem: " << dedicatedSystemMemory << std::endl;
		std::cout << "\tSharedSystemMem: " << sharedSystemMemory << std::endl;
		std::cout << "\tRevision: " << adapterDesc.Revision << std::endl;

		m_adapter3 = (IDXGIAdapter3*)adapter;

		//SafeRelease(&adapter);
	} else {
		// Create warp device if no adapter was found
		m_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
	}

}

void DX12API::createCmdInterfacesAndSwapChain(Win32Window* window) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// 3. Create command queue/allocator/list
	m_directCommandQueue = std::make_unique<CommandQueue>(this, D3D12_COMMAND_LIST_TYPE_DIRECT, L"Main direct command queue");
	
	// 5. Create swap chain
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Flags = (m_tearingSupport) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	IDXGISwapChain1* swapChain1 = nullptr;
	HRESULT hr;
	if (SUCCEEDED(hr = m_factory->CreateSwapChainForHwnd(m_directCommandQueue->get(), *window->getHwnd(), &scDesc, nullptr, nullptr, &swapChain1))) {
		if (SUCCEEDED(hr = swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
			m_swapChain->Release();
		}
	}
	ThrowIfFailed(hr);

	if (m_tearingSupport) {
		// When tearing support is enabled we will handle ALT+Enter key presses in the
		// window message loop rather than let DXGI handle it by calling SetFullscreenState.
		m_factory->MakeWindowAssociation(*window->getHwnd(), DXGI_MWA_NO_ALT_ENTER);
	}

	// No more m_factory using
	Memory::SafeRelease(&m_factory);
}

void DX12API::createEventHandle() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Create an event handle to use for GPU synchronization
	m_eventHandle = CreateEvent(0, false, false, 0);
}

void DX12API::createRenderTargets() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Create descriptor heap for render target views
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetsHeap)));

	// Create resources for the render targets
	m_renderTargetDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	// One RTV for each frame
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++) {
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, cdh);
		m_renderTargets[n]->SetName((std::wstring(L"Render Target ") + std::to_wstring(n)).c_str());
		cdh.ptr += m_renderTargetDescriptorSize;
	}
	// Set first as initial rt
	m_currentRenderTargetCDH = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	m_currentRenderTargetResource = m_renderTargets[0].Get();
}

void DX12API::createGlobalRootSignature() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	D3D12_ROOT_PARAMETER rootParam[GlobalRootParam::SIZE];

	D3D12_ROOT_DESCRIPTOR rootDescCBV_0 = {};
	{
		// CBV_0
		m_globalRootSignatureRegisters["b0"] = { GlobalRootParam::CBV_0, 0 };

		rootDescCBV_0.ShaderRegister = 0;
		rootDescCBV_0.RegisterSpace = 0;

		rootParam[GlobalRootParam::CBV_0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[GlobalRootParam::CBV_0].Descriptor = rootDescCBV_0;
		rootParam[GlobalRootParam::CBV_0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescCBV_1 = {};
	{
		// CBV_1
		m_globalRootSignatureRegisters["b1"] = { GlobalRootParam::CBV_1, 0 };

		rootDescCBV_1.ShaderRegister = 1;
		rootDescCBV_1.RegisterSpace = 0;

		rootParam[GlobalRootParam::CBV_1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[GlobalRootParam::CBV_1].Descriptor = rootDescCBV_1;
		rootParam[GlobalRootParam::CBV_1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescCBV_2 = {};
	{
		// CBV_2
		m_globalRootSignatureRegisters["b2"] = { GlobalRootParam::CBV_2, 0 };

		rootDescCBV_2.ShaderRegister = 2;
		rootDescCBV_2.RegisterSpace = 0;

		rootParam[GlobalRootParam::CBV_2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam[GlobalRootParam::CBV_2].Descriptor = rootDescCBV_2;
		rootParam[GlobalRootParam::CBV_2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	D3D12_DESCRIPTOR_RANGE descRangeSRV_UAV[2];
	{
		// DT_SRV_0TO9_UAV_10TO20
		for (unsigned int i = 0; i <= 9; i++) {
			m_globalRootSignatureRegisters["t" + std::to_string(i)] = { GlobalRootParam::DT_SRV_0TO9_UAV_10TO20, i };
		}
		for (unsigned int i = 10; i <= 20; i++) {
			m_globalRootSignatureRegisters["u" + std::to_string(i)] = { GlobalRootParam::DT_SRV_0TO9_UAV_10TO20, i };
		}

		descRangeSRV_UAV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRangeSRV_UAV[0].NumDescriptors = 10;
		descRangeSRV_UAV[0].BaseShaderRegister = 0;
		descRangeSRV_UAV[0].RegisterSpace = 0;
		descRangeSRV_UAV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		descRangeSRV_UAV[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		descRangeSRV_UAV[1].NumDescriptors = 10;
		descRangeSRV_UAV[1].BaseShaderRegister = 10;
		descRangeSRV_UAV[1].RegisterSpace = 0;
		descRangeSRV_UAV[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_DESCRIPTOR_TABLE dtSrv;
		dtSrv.NumDescriptorRanges = ARRAYSIZE(descRangeSRV_UAV);
		dtSrv.pDescriptorRanges = descRangeSRV_UAV;

		rootParam[GlobalRootParam::DT_SRV_0TO9_UAV_10TO20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[GlobalRootParam::DT_SRV_0TO9_UAV_10TO20].DescriptorTable = dtSrv;
		rootParam[GlobalRootParam::DT_SRV_0TO9_UAV_10TO20].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescSRV_10 = {};
	{
		// SRV_10
		m_globalRootSignatureRegisters["t10"] = { GlobalRootParam::SRV_10, 0 };

		rootDescSRV_10.ShaderRegister = 10;
		rootDescSRV_10.RegisterSpace = 0;

		rootParam[GlobalRootParam::SRV_10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParam[GlobalRootParam::SRV_10].Descriptor = rootDescSRV_10;
		rootParam[GlobalRootParam::SRV_10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescSRV_11 = {};
	{
		// SRV_11
		m_globalRootSignatureRegisters["t11"] = { GlobalRootParam::SRV_11, 0 };

		rootDescSRV_11.ShaderRegister = 11;
		rootDescSRV_11.RegisterSpace = 0;

		rootParam[GlobalRootParam::SRV_11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParam[GlobalRootParam::SRV_11].Descriptor = rootDescSRV_11;
		rootParam[GlobalRootParam::SRV_11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescUAV_0 = {};
	{
		// UAV_0
		m_globalRootSignatureRegisters["u0"] = { GlobalRootParam::UAV_0, 0 };

		rootDescUAV_0.ShaderRegister = 0;
		rootDescUAV_0.RegisterSpace = 0;

		rootParam[GlobalRootParam::UAV_0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParam[GlobalRootParam::UAV_0].Descriptor = rootDescUAV_0;
		rootParam[GlobalRootParam::UAV_0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}
	D3D12_ROOT_DESCRIPTOR rootDescUAV_1 = {};
	{
		// UAV_1
		m_globalRootSignatureRegisters["u1"] = { GlobalRootParam::UAV_1, 0 };

		rootDescUAV_1.ShaderRegister = 1;
		rootDescUAV_1.RegisterSpace = 0;

		rootParam[GlobalRootParam::UAV_1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		rootParam[GlobalRootParam::UAV_1].Descriptor = rootDescUAV_1;
		rootParam[GlobalRootParam::UAV_1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	// Static samplers
	auto& samplerMap = ShaderComponent::Sampler::GetShaderSlotsMap();
	D3D12_STATIC_SAMPLER_DESC* staticSamplerDesc = SAIL_NEW D3D12_STATIC_SAMPLER_DESC[samplerMap.size()];
	unsigned int i = 0;
	for (auto& it : samplerMap) {
		staticSamplerDesc[i] = {};
		staticSamplerDesc[i].MipLODBias = 0.f;
		staticSamplerDesc[i].MaxAnisotropy = 16;
		staticSamplerDesc[i].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		staticSamplerDesc[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		staticSamplerDesc[i].MinLOD = 0.f;
		staticSamplerDesc[i].MaxLOD = FLT_MAX;
		staticSamplerDesc[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		staticSamplerDesc[i].ShaderRegister = it.second.slot;
		staticSamplerDesc[i].RegisterSpace = 0;

		switch (it.second.filter) {
		case Texture::ANISOTROPIC:
			staticSamplerDesc[i].Filter = D3D12_FILTER_ANISOTROPIC;
			break;
		case Texture::LINEAR:
			staticSamplerDesc[i].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case Texture::POINT:
			staticSamplerDesc[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			break;
		default:
			assert(false && "Unimplemented sampler filter found");
			break;
		}

		switch (it.second.addressMode) {
		case Texture::BORDER:
			staticSamplerDesc[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			staticSamplerDesc[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			staticSamplerDesc[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			break;
		case Texture::CLAMP:
			staticSamplerDesc[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplerDesc[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplerDesc[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			break;
		case Texture::MIRROR:
			staticSamplerDesc[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			staticSamplerDesc[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			staticSamplerDesc[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			break;
		case Texture::MIRROR_ONCE:
			staticSamplerDesc[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			staticSamplerDesc[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			staticSamplerDesc[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			break;
		case Texture::WRAP:
			staticSamplerDesc[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplerDesc[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplerDesc[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			break;
		default:
			assert(false && "Unimplemented sampler address mode found");
			break;
		}
		i++;
	}

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = GlobalRootParam::SIZE;
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = samplerMap.size();
	rsDesc.pStaticSamplers = staticSamplerDesc;

	// Serialize and create the actual signature
	ID3DBlob* sBlob;
	ID3DBlob* errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, &errorBlob);
	if (FAILED(hr)) {
		MessageBoxA(0, (char*)errorBlob->GetBufferPointer(), "", 0);
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		ThrowIfFailed(hr);
	}
	ThrowIfFailed(m_device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&m_globalRootSignature)));
	delete staticSamplerDesc;
}

void DX12API::createShaderResources() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Create one big gpu descriptor heap for all cbvs, srvs and uavs
	// TODO: maybe dont hardcode numdescriptors?
	m_cbvSrvUavDescriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 50000, true, true);
}

void DX12API::createDepthStencilResources(Win32Window* window) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsDescriptorHeap)));
	m_dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bufferDesc.Width = window->getWindowWidth();
	bufferDesc.Height = window->getWindowHeight();
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 0;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_device->CreateCommittedResource(
		&DX12Utils::sDefaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer)
	);
	m_depthStencilBuffer->SetName(L"Depth/Stencil Resource Buffer");
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	m_dsvDescHandle = m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DX12API::createViewportAndScissorRect(Win32Window* window) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// 7. Viewport and scissor rect
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (float)window->getWindowWidth();
	m_viewport.Height = (float)window->getWindowHeight();
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = (long)0;
	m_scissorRect.right = (long)window->getWindowWidth();
	m_scissorRect.top = (long)0;
	m_scissorRect.bottom = (long)window->getWindowHeight();
}

void DX12API::nextFrame() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Schedule a signal to notify when the current frame has finished presenting
	m_directQueueFenceValues[m_swapIndex] = m_directCommandQueue->signal();
	m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_swapIndex = 1 - m_swapIndex; // Toggle between 0 and 1
	
	static bool firstFrame = true;
	// Wait until the next frame is ready, don't wait on the first frame as there is nothing to wait for
	if (!firstFrame) {
		m_directCommandQueue->waitOnCPU(m_directQueueFenceValues[m_swapIndex], m_eventHandle);
	}
	firstFrame = false;

	// Get the handle for the current render target used as back buffer
	m_currentRenderTargetCDH = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	m_currentRenderTargetCDH.ptr += m_renderTargetDescriptorSize * m_backBufferIndex;
	m_currentRenderTargetResource = m_renderTargets[m_backBufferIndex].Get();
	// Reset descriptor heap index back to 0 as soon as the SRVs can be overwritten
	// This is to avoid having to find a good point to loop the heap index mid-frame
	// as this would be difficult to calculate (depends on the number of objects being 
	// rendered and how many textures each object has)
	if (m_swapIndex == 0) {
		getMainGPUDescriptorHeap()->setIndex(0);
	}
	// New frame!
	EventSystem::getInstance()->dispatchEvent(NewFrameEvent());
	m_frameCount++;
}

void DX12API::resizeBuffers(UINT width, UINT height) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (width == 0 || height == 0)
		return;
	waitForGPU();

	// Resize swap chain and backbuffers
	for (auto& rt : m_renderTargets) {
		rt.Reset();
	}
	ThrowIfFailed(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, (m_tearingSupport) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));

	// Create resources for the render targets
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	// One RTV for each frame
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++) {
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, cdh);
		m_renderTargets[n]->SetName((std::wstring(L"Render Target ") + std::to_wstring(n)).c_str());
		cdh.ptr += m_renderTargetDescriptorSize;
	}
	// Back buffer index now changes to 0
	// Rotate fence values to avoid infinite stall
	auto tmpFenceVal = m_directQueueFenceValues[0];
	m_directQueueFenceValues[0] = m_directQueueFenceValues[1];
	m_directQueueFenceValues[1] = tmpFenceVal;
	
	m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_swapIndex = m_backBufferIndex % NUM_GPU_BUFFERS;
	m_currentRenderTargetResource = m_renderTargets[m_backBufferIndex].Get();
	m_currentRenderTargetCDH = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	// Recreate the dsv
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 0;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_device->CreateCommittedResource(
		&DX12Utils::sDefaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer)
	);
	m_depthStencilBuffer->SetName(L"Depth/Stencil Resource Buffer");
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Update viewport and scissor rect
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_scissorRect.right = (long)width;
	m_scissorRect.bottom = (long)height;
	
}

void DX12API::setDepthMask(DepthMask setting) { /* Defined the the PSO */ }
void DX12API::setFaceCulling(Culling setting) { /* Defined the the PSO */ }
void DX12API::setBlending(Blending setting) { /* Defined the the PSO */ }

void DX12API::clear(const glm::vec4& color) {
	m_clearColor[0] = color.r;
	m_clearColor[1] = color.g;
	m_clearColor[2] = color.b;
	m_clearColor[3] = color.a;
	
	// DX11
	//FLOAT colorArr[4] = { color.x, color.y, color.z, color.w };
	//// Clear back buffer
	//m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorArr);

	//// Clear depth buffer
	//m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DX12API::present(bool vsync) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = { };
	m_swapChain->Present1((UINT)vsync, (vsync || !m_windowedMode || !m_tearingSupport) ? 0 : DXGI_PRESENT_ALLOW_TEARING, &pp);

	//waitForGPU();
	nextFrame();
}

unsigned int DX12API::getMemoryUsage() const {
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return static_cast<unsigned int>(info.CurrentUsage / (1024 * 1024));
}

unsigned int DX12API::getMemoryBudget() const {
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return static_cast<unsigned int>(info.Budget / (1024 * 1024));
}

ID3D12Device5* DX12API::getDevice() const {
	return m_device.Get();
}

ID3D12RootSignature* DX12API::getGlobalRootSignature() const {
	return m_globalRootSignature.Get();
}

DX12API::RootSignEntry DX12API::getRootSignEntryFromRegister(const std::string& reg) const {
	auto it = m_globalRootSignatureRegisters.find(reg);
	if (it != m_globalRootSignatureRegisters.end()) {
		return it->second;
	}
	Logger::Error("Tried to get root index from a slot that is not bound in the global root signature!");
	return {0, 0};
}

UINT DX12API::getSwapIndex() const {
	return m_swapIndex;
}

UINT DX12API::getFrameIndex() const {
	return m_backBufferIndex;
}

UINT DX12API::getNumGPUBuffers() const {
	return NUM_GPU_BUFFERS;
}

DescriptorHeap* const DX12API::getMainGPUDescriptorHeap() const {
	return m_cbvSrvUavDescriptorHeap.get();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DX12API::getCurrentRenderTargetCDH() const {
	return m_currentRenderTargetCDH;
}

ID3D12Resource* DX12API::getCurrentRenderTargetResource() {
	return m_currentRenderTargetResource;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DX12API::getDsvCDH() const {
	return m_dsvDescHandle;
}

IDXGISwapChain4* const DX12API::getSwapChain() const {
	return m_swapChain.Get();
}

const D3D12_VIEWPORT* DX12API::getViewport() const {
	return &m_viewport;
}

const D3D12_RECT* DX12API::getScissorRect() const {
	return &m_scissorRect;
}

DX12API::CommandQueue* DX12API::getDirectQueue() const {
	return m_directCommandQueue.get();
}

unsigned int DX12API::getFrameCount() const {
	return m_frameCount;
}

#ifdef _DEBUG
void DX12API::beginPIXCapture() const {
	if (m_pixGa) {
		m_pixGa->BeginCapture();
	}
}
void DX12API::endPIXCapture() const {
	if (m_pixGa) {
		m_pixGa->EndCapture();
	}
}
#endif

void DX12API::scheduleResourceForInit(std::function<bool(ID3D12GraphicsCommandList4*)> initFunc) {
	m_resourcesScheduledForInit.emplace_back(initFunc);
}

void DX12API::initResources(ID3D12GraphicsCommandList4* cmdList) {
	auto i = std::begin(m_resourcesScheduledForInit);

	while (i != std::end(m_resourcesScheduledForInit)) {
		auto initFunc = *i;
		// Do some stuff
		if (initFunc(cmdList))
			i = m_resourcesScheduledForInit.erase(i);
		else
			++i;
	}
}

void DX12API::initCommand(Command& cmd, D3D12_COMMAND_LIST_TYPE type, LPCWSTR name) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Create allocators
	cmd.allocators.resize(NUM_SWAP_BUFFERS);
	for (UINT i = 0; i < NUM_SWAP_BUFFERS; i++) {
		ThrowIfFailed(m_device->CreateCommandAllocator(type, IID_PPV_ARGS(&cmd.allocators[i])));
		cmd.allocators[i]->SetName(name);
	}
	// Create command lists
	ThrowIfFailed(m_device->CreateCommandList(0, type, cmd.allocators[0].Get(), nullptr, IID_PPV_ARGS(&cmd.list)));
	cmd.list->SetName(name);
	// Command lists are created in the recording state. Since there is nothing to
	// record right now and the main loop expects it to be closed, we close them
	cmd.list->Close();
}

void DX12API::clearBackBuffer(ID3D12GraphicsCommandList4* cmdList) const {
	// Clear
	cmdList->ClearRenderTargetView(m_currentRenderTargetCDH, m_clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(m_dsvDescHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DX12API::renderToBackBuffer(ID3D12GraphicsCommandList4* cmdList) const {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	cmdList->OMSetRenderTargets(1, &m_currentRenderTargetCDH, true, &m_dsvDescHandle);

	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorRect);
}

void DX12API::prepareToRender(ID3D12GraphicsCommandList4* cmdList) const {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Indicate that the back buffer will be used as render target
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_currentRenderTargetResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void DX12API::prepareToPresent(ID3D12GraphicsCommandList4* cmdList) const {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Indicate that the back buffer will be used to present
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_currentRenderTargetResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

bool DX12API::onResize(WindowResizeEvent& event) {
	if (event.isMinimized()) {
		Logger::Log("minimized!");
	}
	resizeBuffers(event.getWidth(), event.getHeight());
	Logger::Log("dx12 resize ran");
	return true;
}

void DX12API::toggleFullscreen() {

	if (!m_tearingSupport)
		return;

	Win32Window* window = Application::getInstance()->getWindow<Win32Window>();
	HWND hWnd = *window->getHwnd();
	DWORD windowStyle = window->getWindowStyle();

	if (!m_windowedMode) {
		// Restore the window's attributes and size.
		SetWindowLong(hWnd, GWL_STYLE, windowStyle);

		SetWindowPos(
			hWnd,
			HWND_NOTOPMOST,
			m_windowRect.left,
			m_windowRect.top,
			m_windowRect.right - m_windowRect.left,
			m_windowRect.bottom - m_windowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE
		);

		ShowWindow(hWnd, SW_NORMAL);
	} else {
		// Save the old window rect so we can restore it when exiting fullscreen mode.
		GetWindowRect(hWnd, &m_windowRect);

		// Make the window borderless so that the client area can fill the screen.
		SetWindowLong(hWnd, GWL_STYLE, windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreenWindowRect;
		try {
			if (m_swapChain) {
				// Get the settings of the display on which the app's window is currently displayed
				Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;
				ThrowIfFailed(m_swapChain->GetContainingOutput(&pOutput));
				DXGI_OUTPUT_DESC Desc;
				ThrowIfFailed(pOutput->GetDesc(&Desc));
				fullscreenWindowRect = Desc.DesktopCoordinates;
			} else {
				// Fallback to EnumDisplaySettings implementation
				throw std::exception();
			}
		} catch (std::exception& e) {
			UNREFERENCED_PARAMETER(e);

			// Get the settings of the primary display
			DEVMODE devMode = {};
			devMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

			fullscreenWindowRect = {
				devMode.dmPosition.x,
				devMode.dmPosition.y,
				devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
				devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
			};
		}

		SetWindowPos(
			hWnd,
			HWND_TOPMOST,
			fullscreenWindowRect.left,
			fullscreenWindowRect.top,
			fullscreenWindowRect.right,
			fullscreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);


		ShowWindow(hWnd, SW_MAXIMIZE);
	}

	m_windowedMode = !m_windowedMode;
}

void DX12API::waitForGPU() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Waits for the GPU to finish all current tasks in all queues
	// Schedule signals and wait for them
	m_directCommandQueue->waitOnCPU(m_directCommandQueue->signal(), m_eventHandle);
}

UINT64 DX12API::CommandQueue::sFenceValue = 0;
wComPtr<ID3D12Fence1> DX12API::CommandQueue::sFence;

DX12API::CommandQueue::CommandQueue(DX12API* context, D3D12_COMMAND_LIST_TYPE type, LPCWSTR name)
	: m_context(context)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = type;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(context->getDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	m_commandQueue->SetName(name);
	// Create fence
	if (!sFence) {
		ThrowIfFailed(context->getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&sFence)));
	}
}
UINT64 DX12API::CommandQueue::signal() {
	auto fenceVal = sFenceValue++;
	ThrowIfFailed(m_commandQueue->Signal(sFence.Get(), fenceVal));
	return fenceVal;
}
void DX12API::CommandQueue::wait(UINT64 fenceValue) const {
	m_commandQueue->Wait(sFence.Get(), fenceValue);
}
bool DX12API::CommandQueue::waitOnCPU(UINT64 fenceValue, HANDLE eventHandle) const {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (sFence->GetCompletedValue() < fenceValue) {
		ThrowIfFailed(sFence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
		return true;
	}
	return false;
}
ID3D12CommandQueue* DX12API::CommandQueue::get() const {
	return m_commandQueue.Get();
}
UINT64 DX12API::CommandQueue::getCurrentFenceValue() const {
	return sFenceValue;
}
UINT64 DX12API::CommandQueue::getCompletedFenceValue() const {
	return sFence->GetCompletedValue();
}
void DX12API::CommandQueue::reset() {
	m_commandQueue.Reset();
	if (sFence) {
		sFence.Reset();
	}
}
void DX12API::CommandQueue::scheduleSignal(std::function<void(UINT64)> func) {
	m_queuedSignals.emplace_back(func);
}
void DX12API::CommandQueue::executeCommandLists(std::initializer_list<ID3D12CommandList*> cmdLists) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Command lists needs to be closed before sent to this method
	m_commandQueue->ExecuteCommandLists((UINT)cmdLists.size(), cmdLists.begin());
	// Signal and return fence values to scheduled lambdas
	for (auto& func : m_queuedSignals) {
		func(this->signal());
	}
	m_queuedSignals.clear();
}
void DX12API::CommandQueue::executeCommandLists(ID3D12CommandList* const* cmdLists, const int nLists) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Command lists needs to be closed before sent to this method
	m_commandQueue->ExecuteCommandLists(nLists, cmdLists);
	// Signal and return fence values to scheduled lambdas
	for (auto& func : m_queuedSignals) {
		func(this->signal());
	}
	m_queuedSignals.clear();
}