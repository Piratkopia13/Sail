#pragma once

#include <string>
#include "../ResourceFormat.h"

namespace FileLoader {

	class STBImageLoader {

	public:
		STBImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData);
		~STBImageLoader();

	};

}