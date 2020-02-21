#pragma once

#include <d3d12.h>
#include "Sail/api/RenderableTexture.h"
#include "DX12Texture.h"
#include "DX12ATexture.h"

class DX12RenderableTexture : public RenderableTexture, public virtual DX12ATexture {

public:
	DX12RenderableTexture(UINT aaSamples = 1, unsigned int width = 320, unsigned int height = 180, ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8, bool createDepthStencilView = true, bool createOnlyDSV = false, const glm::vec4& clearColor = {0.f, 0.f, 0.f, 0.f}, bool singleBuffer = true, UINT bindFlags = 0, UINT cpuAccessFlags = 0, const std::string& name = "Renderable Texture", unsigned int arraySize = 1);
	~DX12RenderableTexture();

	virtual void begin(void* cmdList = nullptr) override;
	virtual void end(void* cmdList = nullptr) override;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) override;
	virtual void changeFormat(ResourceFormat::TextureFormat newFormat) override;
	virtual void resize(int width, int height) override;

	ID3D12Resource* getResource(int frameIndex = -1) const;
	ID3D12Resource* getDepthResource() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getDepthSrvCDH(int frameIndex = -1) const;
	D3D12_CPU_DESCRIPTOR_HANDLE getRtvCDH(int frameIndex = -1) const;
	D3D12_CPU_DESCRIPTOR_HANDLE getDsvCDH(int frameIndex = -1) const;


private:
	void createTextures();
	void createDepthTextures();
	unsigned int getSwapIndex() const;

private:
	std::vector<wComPtr<ID3D12Resource>> m_depthStencilBuffers;
	DescriptorHeap m_cpuRtvDescHeap;
	DescriptorHeap m_cpuDsvDescHeap;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_rtvHeapCDHs;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dsvHeapCDHs;

	glm::vec4 m_clearColor;
	unsigned int m_numSwapBuffers;
	std::string m_name;
	UINT m_width, m_height;
	bool m_hasDepthTextures;
	DXGI_FORMAT m_format;
	unsigned int m_arraySize;

};