#pragma once

#include <string>
#include "../ResourceFormat.h"

enum ddsktx_format;

namespace FileLoader {

	class DDSKTXImageLoader {

	public:
		DDSKTXImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData);
		~DDSKTXImageLoader();

	private:
		ResourceFormat::TextureFormat convertFormat(ddsktx_format ddsktxFormat) const;
	};

}