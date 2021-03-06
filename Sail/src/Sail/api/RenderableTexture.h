#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

class RenderableTexture {
public:
	enum UsageFlags {
		USAGE_DEFAULT			= 0,
		USAGE_SAMPLING_ACCESS	= 1 << 0,
		USAGE_UNORDERED_ACCESS	= 1 << 1,
		USAGE_GENERAL			= USAGE_SAMPLING_ACCESS | USAGE_UNORDERED_ACCESS
	};

public:
	static RenderableTexture* Create(uint32_t width = 320, uint32_t height = 320, UsageFlags usage = USAGE_DEFAULT, const std::string& name = "Unnamed Renderable Texture", 
		ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8, bool singleBuffer = true, uint32_t arraySize = 1, const glm::vec4& clearColor = glm::vec4(0.f));
	virtual ~RenderableTexture() {}

	virtual void begin(void* cmdList = nullptr) = 0;
	virtual void end(void* cmdList = nullptr) = 0;
	virtual void clear(const glm::vec4& color, void* cmdList = nullptr) = 0;
	virtual void changeFormat(ResourceFormat::TextureFormat newFormat) = 0;
	virtual void resize(int width, int height) = 0;

};