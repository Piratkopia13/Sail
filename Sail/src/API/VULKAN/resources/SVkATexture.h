#pragma once
#include "../SVkAPI.h"

class SVkATexture {
public:
	SVkATexture(bool singleBuffer = true);
	virtual ~SVkATexture();

	const VkImage& getImage(unsigned int swapImageIndex = 0) const;
	const VkImageView& getView(unsigned int swapImageIndex = 0) const;
	uint32_t getNumBuffers() const;

	static VkFormat ConvertToVkFormat(ResourceFormat::TextureFormat format, bool isSRGB);

protected:
	SVkAPI* context;

	std::vector<SVkAPI::ImageAllocation> textureImages;
	std::vector<VkImageView> imageViews;

	bool singleBuffer;
};