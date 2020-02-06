#include "pch.h"
#include "TGALoader.h"

namespace FileLoader {

	TGALoader::TGALoader(const std::string& filename, ResourceFormat::TextureData& textureData) {
		// Load the targa image data into memory.
		bool result = loadTarga(filename, textureData);
		if (!result) {
			Logger::Warning("Texture file \"" + filename + "\" could not be read!");
			result = loadTarga("./res/textures/missing.tga", textureData); // TODO: make this more general, don't load it every time
			assert(result && "Missing texture could not be loaded!");
		}
	}

	TGALoader::~TGALoader() {
	}
	
	bool TGALoader::loadTarga(const std::string& filename, ResourceFormat::TextureData& textureData) {

		textureData.channels = 4;
		textureData.bitsPerChannel = 8;
		textureData.format = ResourceFormat::R8G8B8A8;

		int error, bpp, imageSize, index;
		unsigned int i, j, k;
		FILE* filePtr;
		unsigned int count;
		TargaHeader targaFileHeader;
		unsigned char* targaImage;


		// Open the targa file for reading in binary.
		error = fopen_s(&filePtr, filename.c_str(), "rb");
		if (error != 0) {
			//Logger::Error(strerror(error));
			return false;
		}

		// Read in the file header.
		count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
		if (count != 1) {
			return false;
		}

		// Get the important information from the header.
		textureData.height = (int)targaFileHeader.height;
		textureData.width = (int)targaFileHeader.width;
		bpp = (int)targaFileHeader.bpp;

		// Check that it is 32 bit and not 24 bit.
		if (bpp != 32) {
			Logger::Warning("Tried to load texture with " + std::to_string(bpp) + " bpp, requires 32 bpp");
			return false;
		}

		// Calculate the size of the 32 bit image data.
		imageSize = textureData.width * textureData.height * 4;

		// Allocate memory for the targa image data.
		targaImage = SAIL_NEW unsigned char[imageSize];
		if (!targaImage) {
			return false;
		}

		// Read in the targa image data.
		count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
		if (count != imageSize) {
			return false;
		}

		// Close the file.
		error = fclose(filePtr);
		if (error != 0) {
			return false;
		}

		// Allocate memory for the targa destination data.
		textureData.textureData8bit = SAIL_NEW unsigned char[imageSize];
		if (!textureData.textureData8bit) {
			return false;
		}

		// Initialize the index into the targa destination data array.
		index = 0;

		// Initialize the index into the targa image data.
		k = (textureData.width * textureData.height * 4) - (textureData.width * 4);

		// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
		for (j = 0; j < textureData.height; j++) {
			for (i = 0; i < textureData.width; i++) {
				textureData.textureData8bit[index + 0] = targaImage[k + 2];  // Red.
				textureData.textureData8bit[index + 1] = targaImage[k + 1];  // Green.
				textureData.textureData8bit[index + 2] = targaImage[k + 0];  // Blue
				textureData.textureData8bit[index + 3] = targaImage[k + 3];  // Alpha

																// Increment the indexes into the targa data.
				k += 4;
				index += 4;
			}

			// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
			k -= (textureData.width * 8);
		}

		// Release the targa image data now that it was copied into the destination array.
		delete[] targaImage;
		targaImage = 0;

		return true;

	}

}