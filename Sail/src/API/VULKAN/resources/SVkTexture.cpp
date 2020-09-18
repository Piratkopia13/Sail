#include "pch.h"
#include "SVkTexture.h"
#include "Sail/Application.h"
#include "../SVkUtils.h"

#define DDSKTX_IMPLEMENT
#include "dds-ktx/dds-ktx.h"
#include "Sail/utils/Utils.h"

Texture* Texture::Create(const std::string& filename, bool useAbsolutePath) {
	return SAIL_NEW SVkTexture(filename, useAbsolutePath);
}

SVkTexture::SVkTexture(const std::string& filename, bool useAbsolutePath)
	: Texture(filename)
	, m_filename(filename)
	, m_readyToUse(false)
	, m_isCubeMap(false)
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	auto& allocator = m_context->getVmaAllocator();

	void* texData;
	unsigned int bufferSize;
	unsigned int texWidth;
	unsigned int texHeight;
	VkFormat vkImageFormat;
	VkImageType imageType;
	unsigned int arrayLayers = 1;

	std::vector<std::byte> ddsData;
	if (filename.substr(filename.length() - 3) == "dds") {

		std::string path = (useAbsolutePath) ? filename : TextureData::DEFAULT_TEXTURE_LOCATION + filename;

		ddsData = Utils::readFileBinary(path);
		int size = ddsData.size();
		assert(!ddsData.empty());
		ddsktx_texture_info tc = { 0 };
		ddsktx_error err;
		//GLuint tex = 0;
		if (ddsktx_parse(&tc, ddsData.data(), size, &err)) {
			assert(tc.depth == 1);
			//assert(!(tc.flags & DDSKTX_TEXTURE_FLAG_CUBEMAP));
			assert(tc.num_layers == 1);

			texData = &ddsData[tc.data_offset];
			bufferSize = tc.size_bytes;
			texWidth = tc.width;
			texHeight = tc.height;
			//vkImageFormat = ConvertToVkFormat(ResourceFormat::R16G16B16A16_FLOAT);
			vkImageFormat = VK_FORMAT_BC7_SRGB_BLOCK;
			imageType = VK_IMAGE_TYPE_2D;
			m_isCubeMap = true;
			arrayLayers = 6;

			//Create GPU texture from tc data
			/*glGenTextures(1, &tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(img->gl_target, tex);*/

			//for (int mip = 0; mip < tc.num_mips; mip++) {
			//	ddsktx_sub_data sub_data;
			//	ddsktx_get_sub(&tc, &sub_data, ddsData.data(), size, 0, 0, mip);
			//	// Fill/Set texture sub resource data (mips in this case)
			//	if (ddsktx_format_compressed(tc.format)) {
			//		//glCompressedTexImage2D(..);
			//	} else {
			//		//glTexImage2D(..);
			//	}
			//}

			// Now we can delete file data
			//ddsData.clear();
		} else {
			Logger::Error(err.msg);
		}


	} else {
		// Load file using the resource manager
		auto& data = getTextureData(filename, useAbsolutePath);
		// Texture data will either be in HDR (float values) or not, get the correct one
		texData = (data.getTextureData8bit()) ? (void*)data.getTextureData8bit() : (void*)data.getTextureDataFloat();
		bufferSize = data.getAllocatedMemorySize();
		texWidth = data.getWidth();
		texHeight = data.getHeight();
		vkImageFormat = ConvertToVkFormat(data.getFormat());
		imageType = VK_IMAGE_TYPE_2D;
	}

	// Create cpu visible staging buffer
	{
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_stagingBuffer.buffer, &m_stagingBuffer.allocation, nullptr));
	}

	// Copy texture data to the staging buffer
	void* data;
	vmaMapMemory(allocator, m_stagingBuffer.allocation, &data);
	memcpy(data, texData, (size_t)bufferSize);
	vmaUnmapMemory(allocator, m_stagingBuffer.allocation);

	// Create the image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.extent.width = static_cast<uint32_t>(texWidth);
	imageInfo.extent.height = static_cast<uint32_t>(texHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = vkImageFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	imageInfo.queueFamilyIndexCount = 2;
	imageInfo.pQueueFamilyIndices = m_context->getGraphicsAndCopyQueueFamilyIndices();;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = (m_isCubeMap) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VK_CHECK_RESULT(vmaCreateImage(allocator, &imageInfo, &allocInfo, &m_textureImage.image, &m_textureImage.allocation, nullptr));

	bool isMissingTexture = (m_filename == ResourceManager::MISSING_TEXTURE_NAME || m_filename == ResourceManager::MISSING_TEXTURECUBE_NAME);

	auto uploadCompleteCallback = [&] {
		// Clean up staging buffer after copy is completed
		m_stagingBuffer.destroy();

		m_context->scheduleOnGraphicsQueue([&, vkImageFormat](const VkCommandBuffer& cmd) {
			// Transition image for shader usage (has to be done on the graphics queue)
			SVkUtils::TransitionImageLayout(cmd, m_textureImage.image, vkImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, (m_isCubeMap) ? 6 : 1);
		}, std::bind(&SVkTexture::readyForUseCallback, this));
	};

	// Perform copying on the copy queue if this is NOT the missing texture
	// If it is, using the copy queue causes a minimum of 2 frame delay before texture is ready which screws up the missing texture that needs to be available on first draw call.
	if (isMissingTexture) {
		m_context->scheduleOnGraphicsQueue([&, texWidth, texHeight, vkImageFormat](const VkCommandBuffer& cmd) {
			copyToImage(cmd, vkImageFormat, texWidth, texHeight);

			// Transition image for shader usage (has to be done on the graphics queue)
			SVkUtils::TransitionImageLayout(cmd, m_textureImage.image, vkImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, (m_isCubeMap) ? 6 : 1);
		}, std::bind(&SVkTexture::readyForUseCallback, this));
	} else {
		m_context->scheduleOnCopyQueue([&, texWidth, texHeight, vkImageFormat](const VkCommandBuffer& cmd) {
			copyToImage(cmd, vkImageFormat, texWidth, texHeight);
		}, uploadCompleteCallback);
	}


	// Create the image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_textureImage.image;
	viewInfo.viewType = (m_isCubeMap) ? VK_IMAGE_VIEW_TYPE_CUBE : (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = vkImageFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = (m_isCubeMap) ? 6 : 1;
	VK_CHECK_RESULT(vkCreateImageView(m_context->getDevice(), &viewInfo, nullptr, &m_textureImageView));
	
}

SVkTexture::~SVkTexture() {
	vkDestroyImageView(m_context->getDevice(), m_textureImageView, nullptr);
}

const VkImageView& SVkTexture::getView() const {
	return m_textureImageView;
}

bool SVkTexture::isReadyToUse() const {
	return m_readyToUse;
}

bool SVkTexture::isCubeMap() const {
	return m_isCubeMap;
}

VkFormat SVkTexture::ConvertToVkFormat(ResourceFormat::TextureFormat format) {
	VkFormat vkFormat;
	switch (format) {
	case ResourceFormat::R8:
		vkFormat = VK_FORMAT_R8_SRGB;
		break;
	case ResourceFormat::R8G8:
		vkFormat = VK_FORMAT_R8G8_SRGB;
		break;
	case ResourceFormat::R8G8B8A8:
		vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case ResourceFormat::R16_FLOAT:
		vkFormat = VK_FORMAT_R16_SFLOAT;
		break;
	case ResourceFormat::R16G16_FLOAT:
		vkFormat = VK_FORMAT_R16G16_SFLOAT;
		break;
	case ResourceFormat::R16G16B16A16_FLOAT:
		vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case ResourceFormat::R32G32B32A32_FLOAT:
		vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	default:
		vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	}
	return vkFormat;
}

void SVkTexture::copyToImage(const VkCommandBuffer& cmd, VkFormat vkImageFormat, uint32_t texWidth, uint32_t texHeight) {
	unsigned int layerCount = (m_isCubeMap) ? 6 : 1;
	// Transition image to copy destination
	SVkUtils::TransitionImageLayout(cmd, m_textureImage.image, vkImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount);

	// Copy staging buffer to image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		texWidth,
		texHeight,
		1
	};
	vkCmdCopyBufferToImage(cmd, m_stagingBuffer.buffer, m_textureImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void SVkTexture::readyForUseCallback() {
	// Make sure buffer is destroyed
	// It will always already be destroyed at this point, unless this is the missing texture
	m_stagingBuffer.destroy();

	m_readyToUse = true;
	Logger::Log("Texture ready for use :)");
}
