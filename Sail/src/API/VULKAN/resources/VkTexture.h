#pragma once

#include "Sail/api/Texture.h"
//#include "VkATexture.h"
#include "Sail/resources/TextureData.h"
#include "../VkAPI.h"
//#include "DescriptorHeap.h"
#include <mutex>
#include <dxgiformat.h>

class VkTexture : public Texture {
public:
	VkTexture(const std::string& filename, bool useAbsolutePath = false);
	~VkTexture();

	// Returns true when mip maps have been generated and the texture is ready for usage
	bool hasBeenInitialized() const;
	// Returns true when the texture is uploaded to a default buffer
	bool hasBeenUploaded() const;

	const std::string& getFilename() const;

	static DXGI_FORMAT ConvertToDXGIFormat(ResourceFormat::TextureFormat format);

private:

private:
	std::string m_filename;

	VkAPI* m_context;
	
};