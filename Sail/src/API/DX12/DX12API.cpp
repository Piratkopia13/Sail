#include "pch.h"
#include "DX12API.h"
#include "API/Windows/Win32Window.h"
#include "DX12Utils.h"
#include "resources/DescriptorHeap.h"

const UINT DX12API::NUM_SWAP_BUFFERS = 3;

GraphicsAPI* GraphicsAPI::Create() {
	return SAIL_NEW DX12API();
}

DX12API::DX12API()
	: m_backBufferIndex(0)
	, m_clearColor{0.8f, 0.2f, 0.2f, 1.0f}
{
	m_renderTargets.resize(NUM_SWAP_BUFFERS);
	m_fenceValues.resize(NUM_SWAP_BUFFERS, 0);
}

DX12API::~DX12API() {

	
}

bool DX12API::init(Window* window) {

	auto winWindow = static_cast<Win32Window*>(window);
	createDevice();
	createCmdInterfacesAndSwapChain(winWindow);
	createFenceAndEventHandle();
	createRenderTargets();
	createShaderResources();
	createGlobalRootSignature();
	createDepthStencilResources(winWindow);

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

	OutputDebugString(L"DX12 Initialized.\n");

	return true;

}


void DX12API::createDevice() {

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
	// 3. Create command queue/allocator/list

	// Create direct command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_directCommandQueue)));
	m_directCommandQueue->SetName(L"Direct Command Queue");

	// Create compute command queue
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_computeCommandQueue)));
	m_computeCommandQueue->SetName(L"Compute Command Queue");

	// Create copy command queue
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_copyCommandQueue)));
	m_copyCommandQueue->SetName(L"Copy Command Queue");

	// Create allocators
	m_postCommand.allocators.resize(NUM_SWAP_BUFFERS);
	m_computeCommand.allocators.resize(NUM_SWAP_BUFFERS);
	m_copyCommand.allocators.resize(NUM_SWAP_BUFFERS);
	for (UINT i = 0; i < NUM_SWAP_BUFFERS; i++) {
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postCommand.allocators[i])));
		// TODO: Is this required?
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_computeCommand.allocators[i])));
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_copyCommand.allocators[i])));
	}
	// Create command lists
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_postCommand.allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_postCommand.list)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeCommand.allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_computeCommand.list)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copyCommand.allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_copyCommand.list)));

	// Command lists are created in the recording state. Since there is nothing to
	// record right now and the main loop expects it to be closed, we close them
	m_postCommand.list->Close();
	m_computeCommand.list->Close();
	m_copyCommand.list->Close();

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
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(m_factory->CreateSwapChainForHwnd(m_directCommandQueue.Get(), *window->getHwnd(), &scDesc, nullptr, nullptr, &swapChain1))) {
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
			m_swapChain->Release();
		}
	}

	// No more m_factory using
	Memory::SafeRelease(&m_factory);
}

void DX12API::createFenceAndEventHandle() {
	// 4. Create fence
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	for (UINT i = 0; i < NUM_SWAP_BUFFERS; i++)
		m_fenceValues[i] = 1;
	// Create an event handle to use for GPU synchronization
	m_eventHandle = CreateEvent(0, false, false, 0);

	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_computeQueueFence)));
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_copyQueueFence)));
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_directQueueFence)));
}

void DX12API::createRenderTargets() {
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
	// 8. Create root signature

	// Define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE descRangeSrv[1];
	descRangeSrv[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRangeSrv[0].NumDescriptors = 3;
	descRangeSrv[0].BaseShaderRegister = 0; // register bX
	descRangeSrv[0].RegisterSpace = 0; // register (bX,spaceY)
	descRangeSrv[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// TODO: autogen from other data
	m_globalRootSignatureRegisters["t0"] = GlobalRootParam::DT_SRVS;
	m_globalRootSignatureRegisters["t1"] = GlobalRootParam::DT_SRVS;
	m_globalRootSignatureRegisters["t2"] = GlobalRootParam::DT_SRVS;

	// Create descriptor tables
	D3D12_ROOT_DESCRIPTOR_TABLE dtSrv;
	dtSrv.NumDescriptorRanges = ARRAYSIZE(descRangeSrv);
	dtSrv.pDescriptorRanges = descRangeSrv;

	// Create root descriptors
	D3D12_ROOT_DESCRIPTOR rootDescCBV = {};
	rootDescCBV.ShaderRegister = 0; // TODO make shader shared define
	rootDescCBV.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescCBV2 = {};
	rootDescCBV2.ShaderRegister = 1; // TODO make shader shared define
	rootDescCBV2.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescCBV3 = {};
	rootDescCBV3.ShaderRegister = 2; // TODO make shader shared define
	rootDescCBV3.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescSRVT10 = {};
	rootDescSRVT10.ShaderRegister = 10;
	rootDescSRVT10.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescSRVT11 = {};
	rootDescSRVT11.ShaderRegister = 11;
	rootDescSRVT11.RegisterSpace = 0;

	// TODO: autogen from other data
	m_globalRootSignatureRegisters["b0"] = GlobalRootParam::CBV_TRANSFORM;
	m_globalRootSignatureRegisters["b1"] = GlobalRootParam::CBV_DIFFUSE_TINT;
	m_globalRootSignatureRegisters["b2"] = GlobalRootParam::CBV_CAMERA;

	// Create root parameters
	D3D12_ROOT_PARAMETER rootParam[GlobalRootParam::SIZE];

	rootParam[GlobalRootParam::CBV_TRANSFORM].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[GlobalRootParam::CBV_TRANSFORM].Descriptor = rootDescCBV;
	rootParam[GlobalRootParam::CBV_TRANSFORM].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[GlobalRootParam::CBV_DIFFUSE_TINT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[GlobalRootParam::CBV_DIFFUSE_TINT].Descriptor = rootDescCBV2;
	rootParam[GlobalRootParam::CBV_DIFFUSE_TINT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[GlobalRootParam::CBV_CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[GlobalRootParam::CBV_CAMERA].Descriptor = rootDescCBV3;
	rootParam[GlobalRootParam::CBV_CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParam[GlobalRootParam::DT_SRVS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[GlobalRootParam::DT_SRVS].DescriptorTable = dtSrv;
	rootParam[GlobalRootParam::DT_SRVS].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[2];
	staticSamplerDesc[0] = {};
	staticSamplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc[0].MipLODBias = 0.f;
	staticSamplerDesc[0].MaxAnisotropy = 1;
	staticSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSamplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplerDesc[0].MinLOD = 0.f;
	staticSamplerDesc[0].MaxLOD = FLT_MAX;
	staticSamplerDesc[0].ShaderRegister = 0;
	staticSamplerDesc[0].RegisterSpace = 0;
	staticSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	staticSamplerDesc[1] = staticSamplerDesc[0];
	staticSamplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSamplerDesc[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 2;
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
}


void DX12API::createShaderResources() {
	// Create one big gpu descriptor heap for all cbvs, srvs and uavs
	// TODO: maybe dont hardcode 512 as numdescriptors?
	m_cbvSrvUavDescriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 512, true);
}

void DX12API::createDepthStencilResources(Win32Window* window) {
	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsDescriptorHeap)));

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
	m_dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");
	m_depthStencilBuffer->SetName(L"Depth/Stencil Resource Buffer");
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	m_dsvDescHandle = m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DX12API::nextFrame() {

	UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
	m_directCommandQueue->Signal(m_fence.Get(), currentFenceValue);
	m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex]) {
		//OutputDebugStringA("Waiting\n");
		m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
	/*std::string str = std::to_string(m_backBufferIndex) + " : " + std::to_string(m_fenceValues[m_backBufferIndex]) + "\n";
	OutputDebugStringA(str.c_str());*/

	m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;

	auto frameIndex = getFrameIndex();
	// Get the handle for the current render target used as back buffer
	m_currentRenderTargetCDH = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	m_currentRenderTargetCDH.ptr += m_renderTargetDescriptorSize * frameIndex;
	m_currentRenderTargetResource = m_renderTargets[frameIndex].Get();

}

void DX12API::resizeBuffers(UINT width, UINT height) {
	
}

void DX12API::setDepthMask(DepthMask setting) {

	/*switch (setting) {
		case DepthMask::NO_MASK:		m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnabled, 1);
		break;
		case DepthMask::WRITE_MASK:	m_deviceContext->OMSetDepthStencilState(m_depthStencilStateWriteMask, 1);
		break;
		case DepthMask::BUFFER_DISABLED: m_deviceContext->OMSetDepthStencilState(m_depthStencilStateDisabled, 1);
		break;
	}*/

}

void DX12API::setFaceCulling(Culling setting) {

	/*switch (setting) {
		case Culling::NO_CULLING: m_deviceContext->RSSetState(m_rasterStateNoCulling);
		break;
		case Culling::FRONTFACE:  m_deviceContext->RSSetState(m_rasterStateFrontfaceCulling);
		break;
		case Culling::BACKFACE:	  m_deviceContext->RSSetState(m_rasterStateBackfaceCulling);
		break;
	}*/

}

void DX12API::setBlending(Blending setting) {

	/*switch (setting) {
		case Blending::NO_BLENDING:	m_deviceContext->OMSetBlendState(m_blendStateDisabled, NULL, 0xffffff);
		break;
		case Blending::ALPHA:		m_deviceContext->OMSetBlendState(m_blendStateAlpha, NULL, 0xffffff);
		break;
		case Blending::ADDITIVE:	m_deviceContext->OMSetBlendState(m_blendStateAdditive, NULL, 0xffffff);
		break;
	}*/

}

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

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = { };
	m_swapChain->Present1((UINT)vsync, (vsync) ? 0 : DXGI_PRESENT_ALLOW_TEARING, &pp);

	//waitForGPU(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
	nextFrame();
	
}

unsigned int DX12API::getMemoryUsage() const {
	/*DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return info.CurrentUsage / 1000000;*/
	return -1;
}

unsigned int DX12API::getMemoryBudget() const {
	/*DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return info.Budget / 1000000;*/
	return -1;
}

ID3D12Device5* DX12API::getDevice() const {
	return m_device.Get();
}

ID3D12RootSignature* DX12API::getGlobalRootSignature() const {
	return m_globalRootSignature.Get();
}

UINT DX12API::getRootIndexFromRegister(const std::string& reg) const {
	auto it = m_globalRootSignatureRegisters.find(reg);
	if (it != m_globalRootSignatureRegisters.end()) {
		return it->second;
	}
	Logger::Error("Tried to get root index from a slot that is not bound in the global root signature!");
	return -1;
}

UINT DX12API::getFrameIndex() const {
	return m_backBufferIndex;
}

UINT DX12API::getNumSwapBuffers() const {
	return NUM_SWAP_BUFFERS;
}

DescriptorHeap* const DX12API::getMainGPUDescriptorHeap() const {
	return m_cbvSrvUavDescriptorHeap.get();
}

void DX12API::initCommand(Command& cmd) {
	// Create allocators
	cmd.allocators.resize(NUM_SWAP_BUFFERS);
	for (UINT i = 0; i < NUM_SWAP_BUFFERS; i++) {
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd.allocators[i])));
	}
	// Create command lists
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd.allocators[0].Get(), nullptr, IID_PPV_ARGS(&cmd.list)));
	// Command lists are created in the recording state. Since there is nothing to
	// record right now and the main loop expects it to be closed, we close them
	cmd.list->Close();

	// Reset allocator and command list to accept commands
	/*ThrowIfFailed(cmd.allocators[getFrameIndex()]->Reset());
	ThrowIfFailed(cmd.list->Reset(cmd.allocators[getFrameIndex()].Get(), nullptr));*/
}

void DX12API::executeCommandLists(std::initializer_list<ID3D12CommandList*> cmdLists) const {
	// Command lists needs to be closed before sent to this method
	m_directCommandQueue->ExecuteCommandLists((UINT)cmdLists.size(), cmdLists.begin());
}

void DX12API::renderToBackBuffer(ID3D12GraphicsCommandList4* cmdList) const {
	// Indicate that the back buffer will be used as render target
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_currentRenderTargetResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	cmdList->OMSetRenderTargets(1, &m_currentRenderTargetCDH, true, &m_dsvDescHandle);

	// Clear
	cmdList->ClearRenderTargetView(m_currentRenderTargetCDH, m_clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(m_dsvDescHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorRect);
}

void DX12API::prepareToPresent(ID3D12GraphicsCommandList4* cmdList) const {
	// Indicate that the back buffer will be used to present
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_currentRenderTargetResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void DX12API::resize(UINT width, UINT height) {
	resizeBuffers(width, height);
}

void DX12API::waitForGPU() {
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = m_fenceValues[m_backBufferIndex];
	m_directCommandQueue->Signal(m_fence.Get(), fence);

	//Wait until command queue is done.
	//if (m_fence->GetCompletedValue() < fence) {
	m_fence->SetEventOnCompletion(fence, m_eventHandle);
	WaitForSingleObject(m_eventHandle, INFINITE);
	m_fenceValues[m_backBufferIndex]++;
	//}
	m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}