#pragma once
#include "../api/RenderableTexture.h"

class SSAO {
public:
	SSAO();
	~SSAO();
	// Returns a texture resource to be used as the render target while executing ssao
	RenderableTexture* getRenderTargetTexture();

	uint32_t getRenderTargetWidth() const;
	uint32_t getRenderTargetHeight() const;

	const std::tuple<void*, uint32_t> getKernel() const;
	const std::tuple<void*, uint32_t> getNoise() const;

	void resize(uint32_t width, uint32_t height);
	
private:
	std::unique_ptr<RenderableTexture> m_outputTexture;
	float m_resScale;
	uint32_t m_width;
	uint32_t m_height;
	std::vector<glm::vec4> m_kernel;
	std::vector<glm::vec4> m_noise;
};