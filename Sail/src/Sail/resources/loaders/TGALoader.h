#pragma once

#include <string>
#include "../ResourceFormat.h"
#include "../../utils/Utils.h"

namespace FileLoader {

	class TGALoader {

	public:
		TGALoader(const std::string& filename, ResourceFormat::TextureData& textureData);
		~TGALoader();


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