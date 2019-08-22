#pragma once

#include <string>
#include "../ResourceFormat.h"
#include "../../utils/Utils.h"

namespace FileLoader {

	class TGALoader {

	public:
		TGALoader(std::string filename, ResourceFormat::TextureData& textureData);
		~TGALoader();


	private:
		bool loadTarga(std::string filename, ResourceFormat::TextureData& textureData);

	private:

		struct TargaHeader {
			unsigned char data1[12];
			unsigned short width;
			unsigned short height;
			unsigned char bpp;
			unsigned char data2;
		};

	};

}