#pragma once

#include "Sail/api/RenderableTexture.h"
#include "SVkTexture.h"
//#include "VkATexture.h"

class VkRenderableTexture : public RenderableTexture {

public:
	VkRenderableTexture(UINT aaSamples = 1, unsigned int width = 320, unsigned int height = 180, ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8, bool createDepthStencilView = true, bool createOnlyDSV = false, const glm::vec4& clearColor = {0.f, 0.f, 0.f, 0.f}, bool singleBuffer = true, UINT bindFlags = 0, UINT cpuAccessFlags = 0, const std::string& name = "Renderable Texture", unsigned int arraySize = 1);
	~VkRenderableTexture();

	virtual void begin(void* cmdList = nullptr) override;
	virtual void end(void* cmdList = nullptr) override;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) override;
	virtual void changeFormat(ResourceFormat::TextureFormat newFormat) override;
	virtual void resize(int width, int height) override;

private:

};