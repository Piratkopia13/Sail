#include "pch.h"
#include "DX11API.h"
#include "Win32Window.h"

DXAPI::DXAPI()
	: m_device(nullptr)
	, m_deviceContext(nullptr)
	, m_swapChain(nullptr)
	, m_renderTargetView(nullptr)
{}

DXAPI::~DXAPI() {
	if (m_deviceContext)
		m_deviceContext->ClearState();

	Memory::safeRelease(m_renderTargetView);
	Memory::safeRelease(m_swapChain);
	Memory::safeRelease(m_deviceContext);
	Memory::safeRelease(m_device);
	Memory::safeRelease(m_device3);
	Memory::safeRelease(m_dxgiDevice);
	Memory::safeRelease(m_adapter3);
	Memory::safeRelease(m_depthStencilBuffer);
	Memory::safeRelease(m_depthStencilStateEnabled);
	Memory::safeRelease(m_depthStencilStateWriteMask);
	Memory::safeRelease(m_depthStencilStateDisabled);
	Memory::safeRelease(m_depthStencilView);
	Memory::safeRelease(m_rasterStateBackfaceCulling);
	Memory::safeRelease(m_rasterStateFrontfaceCulling);
	Memory::safeRelease(m_rasterStateBackfaceCullingNoConservative);
	Memory::safeRelease(m_rasterStateNoCulling);
	Memory::safeRelease(m_blendStateAlpha);
	Memory::safeRelease(m_blendStateDisabled);
	Memory::safeRelease(m_blendStateAdditive);
	Memory::safeRelease(m_perf);
}

bool DXAPI::init(Win32Window* window) {

	// Number of samples to uses for anti aliasing
	// Set to 1 for no aa
	m_aaSamples = 1;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1; // Double buffered
	swapDesc.BufferDesc.Width = window->getWindowWidth();
	swapDesc.BufferDesc.Width = window->getWindowHeight();
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = *window->getHwnd();
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = m_aaSamples;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allows alt-enter fullscreen

	HRESULT result;
	for (UINT i = 0; i < numDriverTypes; i++) {

		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[i], NULL, createDeviceFlags, featureLevels,
											   numFeatureLevels, D3D11_SDK_VERSION, &swapDesc, &m_swapChain, &m_device, &m_featureLevel, &m_deviceContext);
		ThrowIfFailed(result);
		if (SUCCEEDED(result)) {
			m_driverType = driverTypes[i];
			ThrowIfFailed(m_device->QueryInterface(__uuidof (ID3D11Device3), (void **)&m_device3));
			
			ThrowIfFailed(m_device->QueryInterface(__uuidof(IDXGIDevice3), (void **)&m_dxgiDevice));
			ThrowIfFailed(m_dxgiDevice->GetAdapter((IDXGIAdapter**)&m_adapter3));
			//ThrowIfFailed(m_device->QueryInterface(__uuidof (IDXGIAdapter3), (void **)&m_adapter3));

			break;
		}
	}

	if (FAILED(result)) {
		OutputDebugString(L"\n Failed to create device and swap chain\n\n");
		return false;
	}

	// Set up directX to break the program when any warning or error occurs
	ID3D11InfoQueue* infoQueue = nullptr;
	m_device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&infoQueue));
	if (infoQueue) {
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

		D3D11_MESSAGE_ID hide[] = {
			D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
			// Add more message IDs here as needed
		};

		D3D11_INFO_QUEUE_FILTER filter;
		memset(&filter, 0, sizeof(filter));
		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		infoQueue->AddStorageFilterEntries(&filter);
		infoQueue->Release();

		Memory::safeRelease(infoQueue);
		infoQueue = nullptr;
	}
	m_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_debug));
	// Performance thing used to put markers in performance profiler data
	m_deviceContext->QueryInterface(__uuidof(m_perf), reinterpret_cast<void**>(&m_perf));


	// Create render target view
	ID3D11Texture2D* backBufferTex = nullptr;
	m_swapChain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTex));
	ThrowIfFailed(m_device->CreateRenderTargetView(backBufferTex, nullptr, &m_renderTargetView));
	Memory::safeRelease(backBufferTex);


	// Set up depth stencil description
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the enabled depth stencil state
	ThrowIfFailed(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStateEnabled));

	// Create the depth stencil state with write masking
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	ThrowIfFailed(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStateWriteMask));
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	// Create the depth stencil state with disbled depth testing
	depthStencilDesc.DepthEnable = false;
	ThrowIfFailed(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStateDisabled));

	// Set the depth stencil state
	m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnabled, 1);

	createDepthStencilBufferAndBind(window->getWindowWidth(), window->getWindowHeight());

	// Set up alpha blending
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(m_device->CreateBlendState(&blendStateDesc, &m_blendStateAlpha));
	blendStateDesc.RenderTarget[0].BlendEnable = false;
	ThrowIfFailed(m_device->CreateBlendState(&blendStateDesc, &m_blendStateDisabled));
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(m_device->CreateBlendState(&blendStateDesc, &m_blendStateAdditive));

	// Setup rasterizer description
	D3D11_RASTERIZER_DESC2 rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	//rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_CW = false;
	rasterDesc.FrontCounterClockwise = m_CW;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.f;

	// Create rasterizer state

	//D3D_FEATURE_LEVEL fl = m_device->GetFeatureLevel();
	D3D11_FEATURE_DATA_D3D11_OPTIONS2 options2Support;
	m_device3->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &options2Support, sizeof(options2Support));

	if (options2Support.ConservativeRasterizationTier == D3D11_CONSERVATIVE_RASTERIZATION_NOT_SUPPORTED) {
		Logger::Warning("Conservative rasterization not supported on this hardware, running without.");
		rasterDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	} else {
		rasterDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON;
	}

	ThrowIfFailed(m_device3->CreateRasterizerState2(&rasterDesc, &m_rasterStateBackfaceCulling));
	rasterDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	ThrowIfFailed(m_device3->CreateRasterizerState2(&rasterDesc, &m_rasterStateBackfaceCullingNoConservative));
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	ThrowIfFailed(m_device3->CreateRasterizerState2(&rasterDesc, &m_rasterStateFrontfaceCulling));
	rasterDesc.CullMode = D3D11_CULL_NONE;
	ThrowIfFailed(m_device3->CreateRasterizerState2(&rasterDesc, &m_rasterStateNoCulling));

	// Set the rasterizer state
	m_deviceContext->RSSetState(m_rasterStateBackfaceCulling);

	// Viewport creation
	m_viewport.Width = static_cast<float>(window->getWindowWidth());
	m_viewport.Height = static_cast<float>(window->getWindowHeight());
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// Bind viewport
	m_deviceContext->RSSetViewports(1, &m_viewport);

	return true;

}

void DXAPI::createDepthStencilBufferAndBind(UINT windowWidth, UINT windowHeight) {

	// Release everything that is set already
	// This is done since this method is called on window resize
	Memory::safeRelease(m_depthStencilBuffer);
	Memory::safeRelease(m_depthStencilView);

	// Set up depth buffer description
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = windowWidth;
	depthBufferDesc.Height = windowHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthBufferDesc.SampleDesc.Count = m_aaSamples;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create texture for depth buffer
	ThrowIfFailed(m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer));

	// Set up depth stencil view description
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = (m_aaSamples == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

	// Bind render target view
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

}

void DXAPI::resizeBuffers(UINT width, UINT height) {
	if (m_swapChain) {

		m_deviceContext->OMSetRenderTargets(0, 0, m_depthStencilView);

		// Release all outstanding references to the swap chain's buffers
		Memory::safeRelease(m_renderTargetView);

		// Preserve the existing buffer count and format
		// Automatically choose the width and height to match the client rect for HWNDs
		ThrowIfFailed(m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

		// Get buffer and create a render-target-view
		ID3D11Texture2D* backBufferTex = nullptr;
		ThrowIfFailed(m_swapChain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTex)));
		ThrowIfFailed(m_device->CreateRenderTargetView(backBufferTex, nullptr, &m_renderTargetView));
		Memory::safeRelease(backBufferTex);

		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Viewport creation
		m_viewport.Width = static_cast<FLOAT>(width);
		m_viewport.Height = static_cast<FLOAT>(height);
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		// Bind viewport
		m_deviceContext->RSSetViewports(1, &m_viewport);

		// Re-create the depth and stencil buffers
		createDepthStencilBufferAndBind(width, height);

	}
}

void DXAPI::setDepthMask(DepthMask setting) {

	switch (setting) {
		case DepthMask::NO_MASK:		m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnabled, 1);
		break;
		case DepthMask::WRITE_MASK:	m_deviceContext->OMSetDepthStencilState(m_depthStencilStateWriteMask, 1);
		break;
		case DepthMask::BUFFER_DISABLED: m_deviceContext->OMSetDepthStencilState(m_depthStencilStateDisabled, 1);
		break;
	}

}

void DXAPI::setFaceCulling(Culling setting) {

	switch (setting) {
		case Culling::NO_CULLING: m_deviceContext->RSSetState(m_rasterStateNoCulling);
		break;
		case Culling::FRONTFACE:  m_deviceContext->RSSetState(m_rasterStateFrontfaceCulling);
		break;
		case Culling::BACKFACE:	  m_deviceContext->RSSetState(m_rasterStateBackfaceCulling);
		break;
	}

}

void DXAPI::setBlending(Blending setting) {

	switch (setting) {
		case Blending::NO_BLENDING:	m_deviceContext->OMSetBlendState(m_blendStateDisabled, NULL, 0xffffff);
		break;
		case Blending::ALPHA:		m_deviceContext->OMSetBlendState(m_blendStateAlpha, NULL, 0xffffff);
		break;
		case Blending::ADDITIVE:	m_deviceContext->OMSetBlendState(m_blendStateAdditive, NULL, 0xffffff);
		break;
	}

}

void DXAPI::clear(const DirectX::SimpleMath::Vector4& color) {
	FLOAT colorArr[4] = { color.x, color.y, color.z, color.w };
	// Clear back buffer
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorArr);

	// Clear depth buffer
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DXAPI::present(bool vsync) {
	m_swapChain->Present(vsync, 0);
}

ID3D11Device* DXAPI::getDevice() const {
	return m_device;
}

ID3D11DeviceContext* DXAPI::getDeviceContext() const {
	return m_deviceContext;
}

ID3D11DepthStencilView* DXAPI::getDepthStencilView() const {
	return m_depthStencilView;
}

UINT DXAPI::getAASamples() {
	return m_aaSamples;
}

UINT64 DXAPI::getMemoryUsage() {
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return info.CurrentUsage / 1000000;
}

UINT64 DXAPI::getMemoryBudget() {
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	m_adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	return info.Budget / 1000000;
}

ID3D11RenderTargetView* const* DXAPI::getBackBufferRTV() const {
	return &m_renderTargetView;
}

ID3DUserDefinedAnnotation* DXAPI::getPerfProfiler() {
	return m_perf;
}

void DXAPI::renderToBackBuffer() const {
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_deviceContext->RSSetViewports(1, &m_viewport);
}

void DXAPI::resize(UINT width, UINT height) {
	resizeBuffers(width, height);
}
