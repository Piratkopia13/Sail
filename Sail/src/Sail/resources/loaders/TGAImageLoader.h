#pragma once

#include <string>
#include "../ResourceFormat.h"
#include "../../utils/Utils.h"

namespace FileLoader {

	class TGAImageLoader {

	public:
		TGAImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData);
		~TGAImageLoader();


	private:
		bool loadTarga(const std::string& filename, ResourceFormat::TextureData& textureData);

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