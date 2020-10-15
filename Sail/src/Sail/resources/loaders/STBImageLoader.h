#pragma once

#include <string>
#include "../ResourceFormat.h"

namespace FileLoader {

	bool STBImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData);

}