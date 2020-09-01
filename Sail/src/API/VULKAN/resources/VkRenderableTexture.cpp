#include "pch.h"
#include "VkRenderableTexture.h"
#include "Sail/Application.h"
#include "Sail/api/Window.h"
//#include "../VkUtils.h"

RenderableTexture* RenderableTexture::Create(unsigned int width, unsigned int height, const std::string& name, ResourceFormat::TextureFormat format, bool createDepthStencilView, bool createOnlyDSV, const glm::vec4& clearColor, unsigned int arraySize, bool singleBuffer) {
	return SAIL_NEW VkRenderableTexture(1, width, height, format, createDepthStencilView, createOnlyDSV, clearColor, singleBuffer, 0U, 0U, name, arraySize);
}

VkRenderableTexture::VkRenderableTexture(UINT aaSamples, unsigned int width, unsigned int height, ResourceFormat::TextureFormat format, bool createDepthStencilView, bool createOnlyDSV, const glm::vec4& clearColor, bool singleBuffer, UINT bindFlags, UINT cpuAccessFlags, const std::string& name, unsigned int arraySize) {
	assert(false);
}

VkRenderableTexture::~VkRenderableTexture() {

}

void VkRenderableTexture::begin(void* cmdList) {
	assert(false);
}

void VkRenderableTexture::end(void* cmdList) {
	// Does nothing
}

void VkRenderableTexture::clear(const glm::vec4& color, void* cmdList) {
	assert(false);
}

void VkRenderableTexture::changeFormat(ResourceFormat::TextureFormat newFormat) {
	assert(false);
}

void VkRenderableTexture::resize(int width, int height) {
	assert(false);
}
