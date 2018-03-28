#include "TGALoader.h"

namespace FileLoader {

	TGALoader::TGALoader(std::string filename, ResourceFormat::TextureData& textureData) {

		bool result;

		// Load the targa image data into memory.
		result = loadTarga(filename, textureData);
		if (!result) {
			Logger::Error("Texture file \"" + filename + "\" could not be read!");
			return;
		}

	}
	TGALoader::~TGALoader() {
	}


	bool TGALoader::loadTarga(std::string filename, ResourceFormat::TextureData& textureData) {

		textureData.channels = 4;

		int error, bpp, imageSize, index;
		unsigned int i, j, k;
		FILE* filePtr;
		unsigned int count;
		TargaHeader targaFileHeader;
		unsigned char* targaImage;


		// Open the targa file for reading in binary.
		error = fopen_s(&filePtr, filename.c_str(), "rb");
		if (error != 0) {
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
			return false;
		}

		// Calculate the size of the 32 bit image data.
		imageSize = textureData.width * textureData.height * 4;

		// Allocate memory for the targa image data.
		targaImage = new unsigned char[imageSize];
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
		textureData.textureData = new unsigned char[imageSize];
		if (!textureData.textureData) {
			return false;
		}

		// Initialize the index into the targa destination data array.
		index = 0;

		// Initialize the index into the targa image data.
		k = (textureData.width * textureData.height * 4) - (textureData.width * 4);

		// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
		for (j = 0; j < textureData.height; j++) {
			for (i = 0; i < textureData.width; i++) {
				textureData.textureData[index + 0] = targaImage[k + 2];  // Red.
				textureData.textureData[index + 1] = targaImage[k + 1];  // Green.
				textureData.textureData[index + 2] = targaImage[k + 0];  // Blue
				textureData.textureData[index + 3] = targaImage[k + 3];  // Alpha

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