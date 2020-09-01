#include "pch.h"
#include "VkTexture.h"
#include "Sail/Application.h"
//#include "../VkUtils.h"
//#include "../shader/VkComputeShaderDispatcher.h"
//#include "../shader/VkPipelineStateObject.h"
#include "DDSTextureLoader/DDSTextureLoader12.cpp"

#include <filesystem>

Texture* Texture::Create(const std::string& filename, bool useAbsolutePath) {
	return SAIL_NEW VkTexture(filename, useAbsolutePath);
}

VkTexture::VkTexture(const std::string& filename, bool useAbsolutePath)
	: Texture(filename)
	, m_filename(filename)
{
	assert(false);
}

VkTexture::~VkTexture() { }

bool VkTexture::hasBeenInitialized() const {
	assert(false);
	return false;
}
bool VkTexture::hasBeenUploaded() const {
	assert(false);
	return false;
}

DXGI_FORMAT VkTexture::ConvertToDXGIFormat(ResourceFormat::TextureFormat format) {
	DXGI_FORMAT dxgiFormat;
	switch (format) {
	case ResourceFormat::R8:
		dxgiFormat = DXGI_FORMAT_R8_UNORM;
		break;
	case ResourceFormat::R8G8:
		dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
		break;
	case ResourceFormat::R8G8B8A8:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case ResourceFormat::R16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16_FLOAT;
		break;
	case ResourceFormat::R16G16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16_FLOAT;
		break;
	case ResourceFormat::R16G16B16A16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case ResourceFormat::R32G32B32A32_FLOAT:
		dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	}
	return dxgiFormat;
}

const std::string& VkTexture::getFilename() const {
	return m_filename;
}
