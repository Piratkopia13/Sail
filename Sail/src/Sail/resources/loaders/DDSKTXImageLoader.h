#pragma once

#include <string>
#include "../ResourceFormat.h"

enum ddsktx_format;

namespace FileLoader {

	bool DDSKTXImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData);
	ResourceFormat::TextureFormat convertFormat(ddsktx_format ddsktxFormat);

}