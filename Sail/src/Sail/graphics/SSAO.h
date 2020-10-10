#pragma once
#include "../api/RenderableTexture.h"

class SSAO {
public:
	SSAO();
	~SSAO();
	// Returns a texture resource to be used as the render target while executing ssao
	RenderableTexture* getRenderTargetTexture();

	unsigned int getRenderTargetWidth() const;
	unsigned int getRenderTargetHeight() const;

	const std::tuple<void*, unsigned int> getKernel() const;
	const std::tuple<void*, unsigned int> getNoise() const;

	void resize(uint32_t width, uint32_t height);
	
private:
	std::unique_ptr<RenderableTexture> m_outputTexture;
	float m_resScale;
	unsigned int m_width;
	unsigned int m_height;
	std::vector<glm::vec4> m_kernel;
	std::vector<glm::vec4> m_noise;
};