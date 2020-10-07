#pragma once

#include <d3d11.h>
#include "Sail/api/RenderableTexture.h"
#include "DX11Texture.h"

class DX11RenderableTexture : public RenderableTexture {

public:
	DX11RenderableTexture(uint32_t width, uint32_t height, UsageFlags usage, ResourceFormat::TextureFormat format,
		bool singleBuffer, bool arraySize, const glm::vec4& clearColor);
	~DX11RenderableTexture();
	
	virtual void begin(void* cmdList = nullptr) override;
	virtual void end(void* cmdList = nullptr) override;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) override;
	virtual void resize(int width, int height) override;

	ID3D11ShaderResourceView* getSRV();
	ID3D11UnorderedAccessView* getUAV();
	ID3D11RenderTargetView* getRTV();
	ID3D11DepthStencilView* getDSV();
	D3D11_VIEWPORT* getViewPort();
	ID3D11Texture2D* getTexture2D();

	void changeFormat(ResourceFormat::TextureFormat newFormat) override;

private:
	void createTextures();

private:
	UINT m_width, m_height;
	DXGI_FORMAT m_format;
	D3D11_VIEWPORT m_viewport;
	DX11Texture* m_texture;
	UINT m_aaSamples;

	ID3D11RenderTargetView* m_rtv;
	ID3D11DepthStencilView* m_dsv;
	ID3D11ShaderResourceView* m_srv;
	ID3D11UnorderedAccessView* m_uav;

	UsageFlags m_usageFlags;

	bool m_isDepthStencil;
	unsigned int m_arraySize;

	//bool m_onlyDSV;
	//UINT m_bindFlags;
	//UINT m_cpuAccessFlags;

	/*ID3D11Texture2D* m_nonMSAAColorTexture2D;
	ID3D11ShaderResourceView* m_nonMSAAColorSRV;
	ID3D11UnorderedAccessView* m_uav;
	ID3D11ShaderResourceView* m_nonMSAADepthSRV;*/

};