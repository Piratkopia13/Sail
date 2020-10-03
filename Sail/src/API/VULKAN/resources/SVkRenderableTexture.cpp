#include "pch.h"
#include "SVkRenderableTexture.h"
#include "Sail/Application.h"
#include "Sail/api/Window.h"
#include "../SVkUtils.h"

RenderableTexture* RenderableTexture::Create(uint32_t width, uint32_t height, UsageFlags usage, const std::string& name, ResourceFormat::TextureFormat format, bool singleBuffer, uint32_t arraySize, const glm::vec4& clearColor)
{
	return SAIL_NEW SVkRenderableTexture(width, height, usage, format, singleBuffer, arraySize, clearColor);
}

SVkRenderableTexture::SVkRenderableTexture(uint32_t width, uint32_t height, UsageFlags usage, ResourceFormat::TextureFormat format,
	bool singleBuffer, unsigned int arraySize, const glm::vec4& clearColor)
	: SVkATexture(singleBuffer)
	, m_width(width)
	, m_height(height)
	, m_usageFlags(usage)
	, m_arraySize(arraySize)
{
	readyToUse = true;
	m_format = SVkTexture::ConvertToVkFormat(format, false);
	m_isDepthStencil = (format == ResourceFormat::DEPTH);

	createTextures();
}

SVkRenderableTexture::~SVkRenderableTexture() {
}

void SVkRenderableTexture::begin(void* cmdList) {
	assert(false && "Not implemented");
}

void SVkRenderableTexture::end(void* cmdList) {
	assert(false && "Not implemented");
}

void SVkRenderableTexture::clear(const glm::vec4& color, void* cmdList) {
	// This is normally already done as part of the render pass
	// It is possible to clear outside the render pass, consider doing this here if needed
	assert(false);
}

void SVkRenderableTexture::changeFormat(ResourceFormat::TextureFormat newFormat) {
	assert(false);
}

void SVkRenderableTexture::resize(int width, int height) {
	vkDeviceWaitIdle(context->getDevice());

	m_width = width;
	m_height = height;

	for (auto& tex : textureImages) {
		tex.destroy();
	}
	for (auto& iv : imageViews) {
		vkDestroyImageView(context->getDevice(), iv, nullptr);
	}
	createTextures();
}

VkFormat SVkRenderableTexture::getFormat() const {
	return m_format;
}

void SVkRenderableTexture::createTextures() {
	auto numBuffers = context->getNumSwapchainImages();	// NOTE: num swap chain images could change after swap chain recreate / window resize, maybe handle this?
	numBuffers = (singleBuffer) ? 1 : numBuffers;

	textureImages.resize(numBuffers);
	imageViews.resize(numBuffers);

	auto& allocator = context->getVmaAllocator();

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (m_usageFlags & UsageFlags::USAGE_SAMPLING_ACCESS) {
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (m_usageFlags & UsageFlags::USAGE_UNORDERED_ACCESS) {
		usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (m_isDepthStencil) {
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	for (int i = 0; i < textureImages.size(); i++) {
		// Create the image
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = m_format;
		imageInfo.extent.width = static_cast<uint32_t>(m_width);
		imageInfo.extent.height = static_cast<uint32_t>(m_height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = m_arraySize;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VK_CHECK_RESULT(vmaCreateImage(allocator, &imageInfo, &allocInfo, &textureImages[i].image, &textureImages[i].allocation, nullptr));

		// Create the image view
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = textureImages[i].image;
		viewInfo.viewType = (m_arraySize > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_format;
		viewInfo.subresourceRange.aspectMask = aspectMask;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = m_arraySize;
		VK_CHECK_RESULT(vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageViews[i]));
	}
}
