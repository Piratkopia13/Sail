#pragma once

#include <d3d11.h>
#include "Sail/api/RenderableTexture.h"
#include "DX11Texture.h"

class DX11RenderableTexture : public RenderableTexture {

public:
	DX11RenderableTexture(UINT aaSamples = 1, unsigned int width = 320, unsigned int height = 180, bool createDepthStencilView = true, bool createOnlyDSV = false, UINT bindFlags = 0, UINT cpuAccessFlags = 0);
	~DX11RenderableTexture();
	
	virtual void begin(void* cmdList = nullptr) override;
	virtual void end(void* cmdList = nullptr) override;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) override;
	virtual void resize(int width, int height) override;

	ID3D11ShaderResourceView** getColorSRV();
	ID3D11ShaderResourceView** getDepthSRV();
	ID3D11RenderTargetView** getRenderTargetView();
	ID3D11DepthStencilView** getDepthStencilView();
	D3D11_VIEWPORT* getViewPort();
	ID3D11Texture2D* getTexture2D();
	ID3D11Texture2D* getDepthTexture2D();

	void changeFormat(ResourceFormat::TextureFormat newFormat) override;

private:
	void createTextures();

private:
	UINT m_width, m_height;
	D3D11_VIEWPORT m_viewport;
	DX11Texture* m_dxColorTexture;
	DX11Texture* m_dxDepthTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	UINT m_aaSamples;
	ID3D11DepthStencilView* m_depthStencilView;
	bool m_hasDepthStencilView;
	bool m_onlyDSV;
	UINT m_bindFlags;
	UINT m_cpuAccessFlags;

	ID3D11Texture2D* m_nonMSAAColorTexture2D;
	ID3D11ShaderResourceView* m_nonMSAAColorSRV;
	ID3D11ShaderResourceView* m_nonMSAADepthSRV;

};