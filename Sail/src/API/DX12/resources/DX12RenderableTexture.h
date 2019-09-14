#pragma once

#include <d3d12.h>
#include "Sail/api/RenderableTexture.h"
#include "DX12Texture.h"
#include "DX12ATexture.h"

class DX12RenderableTexture : public RenderableTexture, public DX12ATexture {

public:
	DX12RenderableTexture(UINT aaSamples = 1, unsigned int width = 320, unsigned int height = 180, bool createDepthStencilView = true, bool createOnlyDSV = false, UINT bindFlags = 0, UINT cpuAccessFlags = 0);
	~DX12RenderableTexture();

	virtual void begin() override;
	virtual void end() override;
	virtual void clear(const glm::vec4& color) override;
	virtual void resize(int width, int height) override;

	ID3D12Resource1* getResource() const;

private:
	void createTextures();

private:
	UINT m_width, m_height;

};