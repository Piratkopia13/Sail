#pragma once

#include "Sail/api/RenderableTexture.h"
#include "SVkTexture.h"
#include "SVkATexture.h"

class SVkRenderableTexture : public RenderableTexture, public SVkATexture {

public:
	SVkRenderableTexture(uint32_t width, uint32_t height, ResourceFormat::TextureFormat format,
		bool singleBuffer = true, unsigned int arraySize = 1, const glm::vec4& clearColor = glm::vec4(1.0f));

	~SVkRenderableTexture();

	virtual void begin(void* cmdList = nullptr) override;
	virtual void end(void* cmdList = nullptr) override;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) override;
	virtual void changeFormat(ResourceFormat::TextureFormat newFormat) override;
	virtual void resize(int width, int height) override;

	VkFormat getFormat() const;

private:
	void createTextures();

private:
	uint32_t m_width;
	uint32_t m_height;
	VkFormat m_format;

	bool m_isDepthStencil;
	unsigned int m_arraySize;

};