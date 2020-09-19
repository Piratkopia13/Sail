#pragma once

#include "Sail/api/Texture.h"
#include "Sail/resources/TextureData.h"
#include "../SVkAPI.h"

class SVkTexture : public Texture {
public:
	SVkTexture(const std::string& filename, bool useAbsolutePath = false);
	~SVkTexture();

	bool isReadyToUse() const; // Returns true when texture is ready for usage
	bool isCubeMap() const;
	const VkImageView& getView() const;

	static VkFormat ConvertToVkFormat(ResourceFormat::TextureFormat format, bool isSRGB);

private:
	void copyToImage(const VkCommandBuffer& cmd, VkFormat vkImageFormat, uint32_t mipLevels, const std::vector<VkExtent3D>& mipExtents, const std::vector<unsigned int>& mipOffsets);
	void generateMipmaps(const VkCommandBuffer& cmd, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkFormat vkImageFormat);
	void readyForUseCallback();

private:
	std::string m_filename;
	SVkAPI* m_context;

	SVkAPI::BufferAllocation m_stagingBuffer;
	SVkAPI::ImageAllocation m_textureImage;
	VkImageView m_textureImageView;
	bool m_isCubeMap;
	bool m_generateMips;

	bool m_readyToUse;
};