#pragma once

#include <d3d11.h>
#include <d3d11_3.h>
#include <dxgi1_4.h>

#include "GraphicsAPI.h"

class DXAPI : public GraphicsAPI {
public:
	DXAPI();
	~DXAPI();

	bool init(Win32Window* window) override;
	void clear(const DirectX::SimpleMath::Vector4& color) override;
	void setDepthMask(DepthMask setting) override;
	void setFaceCulling(Culling setting) override;
	void setBlending(Blending setting) override;
	void present(bool vsync = false) override;

	ID3D11Device* getDevice() const;
	ID3D11DeviceContext* getDeviceContext() const;
	ID3D11DepthStencilView* getDepthStencilView() const;
	UINT getAASamples();
	UINT64 getMemoryUsage();
	UINT64 getMemoryBudget();
	ID3D11RenderTargetView* const* getBackBufferRTV() const;
	ID3DUserDefinedAnnotation* getPerfProfiler();
	void renderToBackBuffer() const;
	// TODO: replace with event
	void resize(UINT width, UINT height);
private:
	void createDepthStencilBufferAndBind(UINT windowWidth, UINT windowHeight);
	void resizeBuffers(UINT width, UINT height);

private:
	// DirectX attributes
	ID3D11Device* m_device;
	IDXGIDevice3* m_dxgiDevice;
	IDXGIAdapter3* m_adapter3;
	ID3D11DeviceContext* m_deviceContext;
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	D3D_DRIVER_TYPE m_driverType;
	D3D_FEATURE_LEVEL m_featureLevel;
	D3D11_VIEWPORT m_viewport;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11ShaderResourceView* m_depthStencilSRV;

	ID3D11DepthStencilState* m_depthStencilStateEnabled;
	ID3D11DepthStencilState* m_depthStencilStateDisabled;
	ID3D11DepthStencilState* m_depthStencilStateWriteMask;

	ID3D11RasterizerState* m_rasterStateBackfaceCulling;
	ID3D11RasterizerState* m_rasterStateFrontfaceCulling;
	ID3D11RasterizerState* m_rasterStateNoCulling;

	ID3D11BlendState* m_blendStateAlpha;
	ID3D11BlendState* m_blendStateDisabled;
	ID3D11BlendState* m_blendStateAdditive;
	ID3D11Debug* m_debug;

	ID3DUserDefinedAnnotation* m_perf;

	UINT m_aaSamples;

};