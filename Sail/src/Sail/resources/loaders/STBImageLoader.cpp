#include "pch.h"
#include "STBImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
// TODO: fix the following defines
//#define STBI_MALLOC(sz)           SAIL_NEW float[sz]
//#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
//#define STBI_FREE(p)              free(p)
#include <stb_image.h>

FileLoader::STBImageLoader::STBImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData) {
	textureData.channels = 4;
	int numChannels;
	
	if (stbi_is_hdr(filename.c_str())) {
		// Load HDR image

		float* data = stbi_loadf(filename.c_str(), (int*)&textureData.width, (int*)&textureData.height, &numChannels, textureData.channels);

		textureData.bitsPerChannel = 32;
		textureData.format = ResourceFormat::R32G32B32A32_FLOAT;

		// Copy the data over to a SAIL_NEW allocated memory region. This is required for the TextureData class to be able to delete the memory when possible
		unsigned int imageSize = textureData.width * textureData.height * textureData.channels;
		textureData.textureDataFloat = SAIL_NEW float[imageSize];
		memcpy(textureData.textureDataFloat, data, imageSize * sizeof(float));

		stbi_image_free(data);
	} else {
		// Load LDR image
		
		stbi_uc* data = stbi_load(filename.c_str(), (int*)&textureData.width, (int*)&textureData.height, &numChannels, textureData.channels);
		//int bpp = stbi_info_from_memory(data, , (int*)&textureData.width, (int*)&textureData.height, &numChannels);

		textureData.bitsPerChannel = 8;
		textureData.format = ResourceFormat::R8G8B8A8;

		// Copy the data over to a SAIL_NEW allocated memory region. This is required for the TextureData class to be able to delete the memory when possible
		unsigned int imageSize = textureData.width * textureData.height * textureData.channels;
		textureData.textureData8bit = SAIL_NEW unsigned char[imageSize];
		memcpy(textureData.textureData8bit, data, imageSize * sizeof(unsigned char));

		stbi_image_free(data);
	}
	
}

FileLoader::STBImageLoader::~STBImageLoader() { }
