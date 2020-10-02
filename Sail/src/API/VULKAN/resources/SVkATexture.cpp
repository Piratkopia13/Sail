#include "pch.h"
#include "SVkATexture.h"

SVkATexture::SVkATexture(bool singleBuffer)
	: singleBuffer(singleBuffer)
	, readyToUse(false)
{
	context = Application::getInstance()->getAPI<SVkAPI>();
}

SVkATexture::~SVkATexture() {
	for (auto& iv : imageViews) {
		vkDestroyImageView(context->getDevice(), iv, nullptr);
	}
}

bool SVkATexture::isReadyToUse() const {
	return readyToUse;
}

const VkImage& SVkATexture::getImage(unsigned int swapImageIndex) const {
	unsigned int i = (singleBuffer) ? 0 : swapImageIndex;
	return textureImages[i].image;
}

const VkImageView& SVkATexture::getView(unsigned int swapImageIndex) const {
	unsigned int i = (singleBuffer) ? 0 : swapImageIndex;
	return imageViews[i];
}

uint32_t SVkATexture::getNumBuffers() const {
	return textureImages.size();
}

VkFormat SVkATexture::ConvertToVkFormat(ResourceFormat::TextureFormat format, bool isSRGB) {
	switch (format) {
	case ResourceFormat::R8:
		return (isSRGB) ? VK_FORMAT_R8_SRGB : VK_FORMAT_R8_UNORM;
		break;
	case ResourceFormat::R8G8:
		return (isSRGB) ? VK_FORMAT_R8G8_SRGB : VK_FORMAT_R8G8_UNORM;
		break;
	case ResourceFormat::R8G8B8A8:
		return (isSRGB) ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case ResourceFormat::R16_FLOAT:
		return VK_FORMAT_R16_SFLOAT;
		break;
	case ResourceFormat::R16G16_FLOAT:
		return VK_FORMAT_R16G16_SFLOAT;
		break;
	case ResourceFormat::R16G16B16A16_FLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case ResourceFormat::R32G32B32A32_FLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case ResourceFormat::BC3:
		return (isSRGB) ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case ResourceFormat::BC5:
		return VK_FORMAT_BC5_UNORM_BLOCK;
		break;
	case ResourceFormat::BC7:
		return (isSRGB) ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
		break;
	case ResourceFormat::DEPTH:
		return Application::getInstance()->getAPI<SVkAPI>()->getDepthFormat();
		break;
	}

	assert(false && "Format missing from convert method");
	return VK_FORMAT_R8G8B8A8_UNORM;
}
