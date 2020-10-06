#include "pch.h"
#include "SVkTexture.h"
#include "Sail/Application.h"
#include "../SVkUtils.h"

#include "Sail/utils/Utils.h"
#include "SVkATexture.h"

Texture* Texture::Create(const std::string& filename, bool useAbsolutePath) {
	return SAIL_NEW SVkTexture(filename, useAbsolutePath);
}

SVkTexture::SVkTexture(const std::string& filename, bool useAbsolutePath)
	: Texture(filename)
	, SVkATexture(true)
	, m_filename(filename)
	, m_generateMips(false)
{
	auto& allocator = context->getVmaAllocator();

	textureImages.resize(1);
	imageViews.resize(1);

	void* texData;
	unsigned int bufferSize;
	unsigned int texWidth;
	unsigned int texHeight;
	VkFormat vkImageFormat;
	VkImageType imageType;
	int mipLevels;
	std::vector<VkExtent3D> mipExtents;
	std::vector<unsigned int> mipOffsets;

	{
		// Load file using the resource manager
		auto& data = getTextureData(filename, useAbsolutePath);
		texData = data.getData();
		bufferSize = data.getAllocatedMemorySize();
		texWidth = data.getWidth();
		texHeight = data.getHeight();
		vkImageFormat = SVkATexture::ConvertToVkFormat(data.getFormat(), data.isSRGB());
		imageType = VK_IMAGE_TYPE_2D;
		texIsCubeMap = data.isCubeMap();

		mipLevels = data.getMipLevels();
		for (auto& extent : data.getMipExtents()) {
			mipExtents.push_back({(uint32_t)extent.x, (uint32_t)extent.y, 1});
		}
		mipOffsets = data.getMipOffsets();

		if (mipLevels == -1) {
			m_generateMips = true;
			mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
			mipExtents.push_back({ (uint32_t)texWidth, (uint32_t)texHeight, 1 });
			mipOffsets.emplace_back(0);
		}

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
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = (texIsCubeMap) ? 6 : 1;
	imageInfo.format = vkImageFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (m_generateMips) {
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	imageInfo.queueFamilyIndexCount = 2;
	imageInfo.pQueueFamilyIndices = context->getGraphicsAndCopyQueueFamilyIndices();;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = (texIsCubeMap) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VK_CHECK_RESULT(vmaCreateImage(allocator, &imageInfo, &allocInfo, &textureImages[0].image, &textureImages[0].allocation, nullptr));

	bool isMissingTexture = (m_filename == ResourceManager::MISSING_TEXTURE_NAME || m_filename == ResourceManager::MISSING_TEXTURECUBE_NAME);

	auto uploadCompleteCallback = [&, mipExtents, mipLevels, vkImageFormat] {
		// Clean up staging buffer after copy is completed
		m_stagingBuffer.destroy();

		context->scheduleOnGraphicsQueue([&, mipExtents, mipLevels, vkImageFormat](const VkCommandBuffer& cmd) {

			// Generate mips if m_generateMips is set and then transition to shader usage, otherwise just transition
			generateMipmaps(cmd, mipExtents[0].width, mipExtents[0].height, mipLevels, vkImageFormat);

		}, std::bind(&SVkTexture::readyForUseCallback, this));
	};

	// Perform copying on the copy queue if this is NOT the missing texture
	// If it is, using the copy queue causes a minimum of 2 frame delay before texture is ready which screws up the missing texture that needs to be available on first draw call.
	if (isMissingTexture) {
		context->scheduleOnGraphicsQueue([&, vkImageFormat, mipLevels, mipExtents, mipOffsets](const VkCommandBuffer& cmd) {
			
			copyToImage(cmd, vkImageFormat, mipLevels, mipExtents, mipOffsets);
			// Generate mips if m_generateMips is set and then transition to shader usage, otherwise just transition
			generateMipmaps(cmd, mipExtents[0].width, mipExtents[0].height, mipLevels, vkImageFormat);

		}, std::bind(&SVkTexture::readyForUseCallback, this));
	} else {
		context->scheduleOnCopyQueue([&, vkImageFormat, mipLevels, mipExtents, mipOffsets](const VkCommandBuffer& cmd) {
			copyToImage(cmd, vkImageFormat, mipLevels, mipExtents, mipOffsets);
		}, uploadCompleteCallback);
	}

	// Create the image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImages[0].image;
	viewInfo.viewType = (texIsCubeMap) ? VK_IMAGE_VIEW_TYPE_CUBE : (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = vkImageFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = (texIsCubeMap) ? 6 : 1;
	VK_CHECK_RESULT(vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageViews[0]));
	
}

SVkTexture::~SVkTexture() {
}

void SVkTexture::copyToImage(const VkCommandBuffer& cmd, VkFormat vkImageFormat, uint32_t mipLevels, const std::vector<VkExtent3D>& mipExtents, const std::vector<unsigned int>& mipOffsets) {
	unsigned int layerCount = (texIsCubeMap) ? 6 : 1;
	// Transition image to copy destination
	SVkUtils::TransitionImageLayout(cmd, textureImages[0].image, vkImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount, mipLevels);

	if (m_generateMips) {
		// If mips will be generated, we only have data for the first mip level
		mipLevels = 1;
	}

	// Copy staging buffer to image
	std::vector<VkBufferImageCopy> regions;
	regions.reserve(mipLevels);
	for (uint32_t mip = 0; mip < mipLevels; mip++) {
		auto& region = regions.emplace_back();
		region.bufferOffset = mipOffsets[mip];
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = mipExtents[mip];
	}

	vkCmdCopyBufferToImage(cmd, m_stagingBuffer.buffer, textureImages[0].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, regions.data());
}

// Has to be submitted on a queue with graphics capability (since it uses blit)
void SVkTexture::generateMipmaps(const VkCommandBuffer& cmd, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkFormat vkImageFormat) {
	if (m_generateMips) {

		static bool checkSupport = true;
		if (checkSupport) {
			// Check if image format supports linear blitting
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(context->getPhysicalDevice(), vkImageFormat, &formatProperties);
			if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				Logger::Error("Texture image format does not support linear blitting!");
			}

			checkSupport = false;
		}
	
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = textureImages[0].image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(cmd,
				textureImages[0].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				textureImages[0].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

	} else {
		// Mip maps are read from the file and not generated

		// Transition image for shader usage (has to be done on the graphics queue)
		SVkUtils::TransitionImageLayout(cmd, textureImages[0].image, vkImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, (texIsCubeMap) ? 6 : 1, mipLevels);
	}

}

void SVkTexture::readyForUseCallback() {
	// Make sure buffer is destroyed
	// It will always already be destroyed at this point, unless this is the missing texture
	m_stagingBuffer.destroy();

	readyToUse = true;
	Logger::Log("Texture ready for use :)");
}
