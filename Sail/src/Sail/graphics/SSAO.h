#pragma once
#include "../api/RenderableTexture.h"
#include "../events/Events.h"

class SSAO : public IEventListener {
public:
	SSAO();
	~SSAO();
	// Returns a texture resource to be used as the render target while executing ssao
	RenderableTexture* getRenderTargetTexture();

	unsigned int getRenderTargetWidth() const;
	unsigned int getRenderTargetHeight() const;

	const std::tuple<void*, unsigned int> getKernel() const;
	const std::tuple<void*, unsigned int> getNoise() const;
	
	bool onEvent(Event& event) override;

private:
	std::unique_ptr<RenderableTexture> m_outputTexture;
	float m_resScale;
	unsigned int m_width;
	unsigned int m_height;
	std::vector<glm::vec4> m_kernel;
	std::vector<glm::vec4> m_noise;
};