#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

class RenderableTexture {
public:
	static RenderableTexture* Create(unsigned int width = 320, unsigned int height = 180, const std::string& name = "Unnamed Renderable Texture", ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8, bool createDepthStencilView = false, bool createOnlyDSV = false, const glm::vec4& clearColor = {0.f, 0.f, 0.f, 0.f}, unsigned int arraySize = 1, bool singleBuffer = true);
	virtual ~RenderableTexture() {}

	virtual void begin(void* cmdList = nullptr) = 0;
	virtual void end(void* cmdList = nullptr) = 0;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) = 0;
	virtual void changeFormat(ResourceFormat::TextureFormat newFormat) = 0;
	virtual void resize(int width, int height) = 0;

};