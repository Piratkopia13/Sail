#pragma once

#include "Sail/api/Texture.h"
#include "SVkATexture.h"
#include "Sail/resources/TextureData.h"
#include "../SVkAPI.h"

class SVkTexture : public Texture, public SVkATexture {
public:
	SVkTexture(const std::string& filename, bool useAbsolutePath = false);
	~SVkTexture();

private:
	void copyToImage(const VkCommandBuffer& cmd, VkFormat vkImageFormat, uint32_t mipLevels, const std::vector<VkExtent3D>& mipExtents, const std::vector<unsigned int>& mipOffsets);
	void generateMipmaps(const VkCommandBuffer& cmd, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkFormat vkImageFormat);
	void readyForUseCallback();

private:
	std::string m_filename;

	SVkAPI::BufferAllocation m_stagingBuffer;
	
	bool m_generateMips;
};